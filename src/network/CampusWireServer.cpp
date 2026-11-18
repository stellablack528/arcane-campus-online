#include "network/CampusWireServer.hpp"

#include <QTcpSocket>
#include <QTimer>
#include <utility>

namespace arcane::network {
namespace {
constexpr int kMaxPeers = 32;
constexpr int kHandshakeTimeoutMs = 5000;
}

CampusWireServer::CampusWireServer(QObject* parent) : QObject(parent)
{
    connect(&server_, &QTcpServer::newConnection, this, [this] {
        while (auto* socket = server_.nextPendingConnection()) {
            if (peers_.size() >= kMaxPeers) {
                send(socket, {{"type", "error"}, {"text", "校园联机人数已满，请稍后重试。"}});
                connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
                socket->disconnectFromHost();
                continue;
            }
            socket->setReadBufferSize(wire::readBufferBytes);
            peers_.insert(socket, {});
            connect(socket, &QTcpSocket::readyRead, this, [this, socket] { receive(socket); });
            connect(socket, &QTcpSocket::disconnected, this, [this, socket] {
                const auto peer = peers_.take(socket);
                if (!peer.name.isEmpty()) announce(peer, false, socket);
                socket->deleteLater();
            });
            QTimer::singleShot(kHandshakeTimeoutMs, socket, [this, socket] {
                const auto peer = peers_.constFind(socket);
                if (peer != peers_.cend() && peer->name.isEmpty()) socket->abort();
            });
        }
    });
}

CampusWireServer::~CampusWireServer()
{
    // Stop socket callbacks before the peer map is destroyed.
    for (auto* socket : peers_.keys()) QObject::disconnect(socket, nullptr, this, nullptr);
}

bool CampusWireServer::listen(const QHostAddress& address, quint16 port)
{
    return server_.listen(address, port);
}

quint16 CampusWireServer::port() const { return server_.serverPort(); }

bool CampusWireServer::send(QTcpSocket* socket, const QJsonObject& event)
{
    if (wire::writeFrame(*socket, event)) return true;
    // Closing immediately can emit disconnected() and mutate peers_ during a broadcast.
    QMetaObject::invokeMethod(socket, [socket] { socket->abort(); }, Qt::QueuedConnection);
    return false;
}

void CampusWireServer::broadcast(const wire::Room& room, const QJsonObject& event, QTcpSocket* except)
{
    for (auto it = peers_.cbegin(); it != peers_.cend(); ++it)
        if (it.key() != except && !it->name.isEmpty() && it->room == room) send(it.key(), event);
}

void CampusWireServer::announce(const Peer& peer, bool entering, QTcpSocket* except)
{
    broadcast(peer.room, {{"type", "presence"}, {"name", peer.name}, {"location", peer.room.scene},
                          {"text", entering ? "来到了当前场景。" : "离开了当前场景。"}}, except);
}

void CampusWireServer::receive(QTcpSocket* socket)
{
    auto peer = peers_.find(socket);
    if (peer == peers_.end()) return;
    QJsonObject request;
    while (socket->state() == QAbstractSocket::ConnectedState) {
        switch (wire::readFrame(*socket, request)) {
        case wire::ReadResult::Incomplete: return;
        case wire::ReadResult::TooLarge: socket->abort(); return;
        case wire::ReadResult::Invalid: continue;
        case wire::ReadResult::Message:
            if (!handleRequest(socket, peer.value(), request)) return;
            break;
        }
    }
}

bool CampusWireServer::handleRequest(QTcpSocket* socket, Peer& peer, const QJsonObject& request)
{
    const auto type = request.value("type").toString();
    if (peer.name.isEmpty()) {
        if (type != QLatin1String("hello")) return true;
        const auto reject = [socket](const QString& reason, const QString& text) {
            send(socket, {{"type", "error"}, {"code", reason}, {"text", text}});
            socket->disconnectFromHost();
            return false;
        };
        if (request.value("version").toInt() != wire::version)
            return reject(QStringLiteral("version_mismatch"), QStringLiteral("联机版本不一致，请更新客户端和服务端。"));
        const auto name = request.value("name").toString().trimmed();
        const wire::Room room{request.value("era").toString(), request.value("location").toString()};
        if (name.isEmpty() || name.size() > wire::maxNickname || !wire::knownEra(room.era) || !wire::knownScene(room.scene))
            return reject(QStringLiteral("invalid_identity"), QStringLiteral("无法加入：昵称、时代或地点无效。"));
        for (const auto& other : std::as_const(peers_))
            if (other.room.era == room.era && other.name == name)
                return reject(QStringLiteral("name_in_use"), QStringLiteral("本时代已有同名角色在线。"));

        if (!send(socket, {{"type", "welcome"}, {"version", wire::version}, {"era", room.era},
                            {"text", "已连接校园。同场景可交谈，/owl 昵称 内容 可在同一时代跨场景送信。"}}))
            return false;
        peer = {name, room};
        announce(peer, true, socket);
        return true;
    }

    if (type == QLatin1String("move")) {
        const auto scene = request.value("location").toString();
        if (!wire::knownScene(scene) || scene == peer.room.scene) return true;
        announce(peer, false, socket);
        peer.room.scene = scene; // Moves cannot change the era fixed during hello.
        announce(peer, true, socket);
    } else if (type == QLatin1String("say")) {
        const auto text = request.value("text").toString().trimmed();
        if (!text.isEmpty() && text.size() <= wire::maxText)
            broadcast(peer.room, {{"type", "say"}, {"name", peer.name},
                                  {"location", peer.room.scene}, {"text", text}}, socket);
    } else if (type == QLatin1String("owl")) {
        const auto target = request.value("target").toString().trimmed();
        const auto text = request.value("text").toString().trimmed();
        if (target.isEmpty() || target.size() > wire::maxNickname || text.isEmpty() || text.size() > wire::maxText)
            return true;
        bool queued = false;
        for (auto it = peers_.cbegin(); it != peers_.cend(); ++it) {
            if (it->room.era != peer.room.era || it->name != target) continue;
            queued = send(it.key(), {{"type", "owl"}, {"name", peer.name}, {"text", text}});
            break;
        }
        return send(socket, {{"type", queued ? "owl_sent" : "error"},
                             {"text", queued ? "猫头鹰已发送。" : "本时代没有找到可收信的在线角色。"}});
    }
    return true;
}
} // namespace arcane::network
