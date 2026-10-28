#include "network/CampusWireServer.hpp"

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>

namespace arcane::network {
namespace {
constexpr qsizetype kMaxFrame = 4096;

bool knownLocation(const QString& id)
{
    return id == QLatin1String("charms_classroom")
        || id == QLatin1String("great_hall")
        || id == QLatin1String("library")
        || id == QLatin1String("courtyard")
        || id == QLatin1String("house_tables")
        || id == QLatin1String("transfiguration_classroom")
        || id == QLatin1String("hogsmeade");
}
}

CampusWireServer::CampusWireServer(QObject* parent) : QObject(parent)
{
    connect(&server_, &QTcpServer::newConnection, this, [this] {
        while (auto* socket = server_.nextPendingConnection()) {
            peers_.insert(socket, {});
            connect(socket, &QTcpSocket::readyRead, this, [this, socket] { receive(socket); });
            connect(socket, &QTcpSocket::disconnected, this, [this, socket] {
                const auto peer = peers_.take(socket);
                if (!peer.name.isEmpty())
                    broadcast(peer.location, {{"type", "presence"}, {"name", peer.name},
                                              {"text", "离开了当前场景。"}});
                socket->deleteLater();
            });
        }
    });
}

CampusWireServer::~CampusWireServer()
{
    // QTcpServer owns accepted sockets and destroys them after this body.
    // Disconnect first so their teardown cannot call into a destroyed peer map.
    for (auto* socket : peers_.keys())
        QObject::disconnect(socket, nullptr, this, nullptr);
    peers_.clear();
}

bool CampusWireServer::listen(const QHostAddress& address, quint16 port)
{
    return server_.listen(address, port);
}

quint16 CampusWireServer::port() const { return server_.serverPort(); }

void CampusWireServer::send(QTcpSocket* socket, const QJsonObject& event)
{
    socket->write(QJsonDocument(event).toJson(QJsonDocument::Compact) + '\n');
}

void CampusWireServer::broadcast(const QString& location, const QJsonObject& event,
                                 QTcpSocket* except)
{
    for (auto it = peers_.cbegin(); it != peers_.cend(); ++it)
        if (it.key() != except && !it.value().name.isEmpty()
            && (location.isEmpty() || it.value().location == location))
            send(it.key(), event);
}

void CampusWireServer::receive(QTcpSocket* socket)
{
    auto& peer = peers_[socket];
    peer.pending += socket->readAll();
    if (peer.pending.size() > kMaxFrame && !peer.pending.contains('\n')) {
        socket->disconnectFromHost();
        return;
    }
    while (true) {
        const auto end = peer.pending.indexOf('\n');
        if (end < 0) break;
        if (end > kMaxFrame) { socket->disconnectFromHost(); return; }
        const auto frame = peer.pending.left(end);
        peer.pending.remove(0, end + 1);
        QJsonParseError error;
        const auto document = QJsonDocument::fromJson(frame, &error);
        if (error.error != QJsonParseError::NoError || !document.isObject()) continue;
        const auto request = document.object();
        const auto type = request.value("type").toString();
        if (type == QLatin1String("hello")) {
            if (!peer.name.isEmpty()) continue;
            const auto name = request.value("name").toString().trimmed();
            const auto location = request.value("location").toString();
            if (name.isEmpty() || name.size() > 32 || !knownLocation(location)) continue;
            for (const auto& other : peers_)
                if (other.name == name) { send(socket, {{"type", "error"}, {"text", "昵称已在线。"}}); socket->disconnectFromHost(); return; }
            peer.name = name;
            peer.location = location;
            send(socket, {{"type", "welcome"}, {"text", "已连接校园。同场景可交谈，/owl 昵称 内容 可跨场景送信。"}});
            broadcast(location, {{"type", "presence"}, {"name", name}, {"text", "来到了当前场景。"}}, socket);
            continue;
        }
        if (peer.name.isEmpty()) continue;
        if (type == QLatin1String("move")) {
            const auto location = request.value("location").toString();
            if (!knownLocation(location)) continue;
            const auto old = peer.location;
            peer.location = location;
            broadcast(old, {{"type", "presence"}, {"name", peer.name}, {"text", "离开了当前场景。"}}, socket);
            broadcast(location, {{"type", "presence"}, {"name", peer.name}, {"text", "来到了当前场景。"}}, socket);
            send(socket, {{"type", "moved"}, {"location", location}});
        } else if (type == QLatin1String("say")) {
            const auto message = request.value("text").toString().trimmed();
            if (message.isEmpty() || message.size() > 500) continue;
            broadcast(peer.location, {{"type", "say"}, {"name", peer.name},
                                      {"location", peer.location}, {"text", message}}, socket);
        } else if (type == QLatin1String("owl")) {
            const auto target = request.value("target").toString().trimmed();
            const auto message = request.value("text").toString().trimmed();
            if (message.isEmpty() || message.size() > 500) continue;
            bool delivered = false;
            for (auto it = peers_.cbegin(); it != peers_.cend(); ++it) {
                if (it.value().name != target) continue;
                send(it.key(), {{"type", "owl"}, {"name", peer.name}, {"text", message}});
                delivered = true;
                break;
            }
            send(socket, {{"type", delivered ? "owl_sent" : "error"},
                          {"text", delivered ? "猫头鹰已送达。" : "没有找到在线的收信人。"}});
        }
    }
    if (peer.pending.size() > kMaxFrame) socket->disconnectFromHost();
}
} // namespace arcane::network
