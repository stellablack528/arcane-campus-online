#include "network/CampusWireClient.hpp"

#include "network/CampusWireProtocol.hpp"
#include <QSignalBlocker>
#include <algorithm>

namespace arcane::network {

CampusWireClient::CampusWireClient(QObject* parent) : QObject(parent)
{
    socket_.setReadBufferSize(wire::readBufferBytes);
    retry_.setSingleShot(true);
    handshakeDeadline_.setSingleShot(true);
    connect(&retry_, &QTimer::timeout, this, &CampusWireClient::tryConnect);
    connect(&handshakeDeadline_, &QTimer::timeout, this, [this] {
        socket_.abort();
        retryAfterDisconnect();
    });
    connect(&socket_, &QTcpSocket::connected, this, [this] {
        helloLocation_ = location_;
        send({{"type", "hello"}, {"version", wire::version}, {"era", era_},
              {"name", name_}, {"location", helloLocation_}});
    });
    connect(&socket_, &QTcpSocket::readyRead, this, &CampusWireClient::receive);
    connect(&socket_, &QTcpSocket::disconnected, this, &CampusWireClient::retryAfterDisconnect);
    connect(&socket_, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
        // A remote close may still have a final protocol error to read before disconnected().
        if (error != QAbstractSocket::RemoteHostClosedError) retryAfterDisconnect();
    });
}

CampusWireClient::~CampusWireClient() { resetConnection(); }

void CampusWireClient::setEra(const QString& era)
{
    if (era == era_) return;
    resetConnection();
    era_ = era;
    tryConnect();
}

void CampusWireClient::join(const QString& name, const QString& location)
{
    const auto nickname = name.trimmed();
    if (name_ == nickname) {
        move(location);
        return;
    }
    resetConnection();
    name_ = nickname;
    location_ = wire::canonicalScene(location);
    tryConnect();
}

void CampusWireClient::move(const QString& location)
{
    const auto scene = wire::canonicalScene(location);
    if (!wire::knownScene(scene) || location_ == scene) return;
    location_ = scene; // Keep the latest scene even while disconnected or waiting for welcome.
    if (connected()) send({{"type", "move"}, {"location", location_}});
    else tryConnect();
}

void CampusWireClient::tryConnect()
{
    if (blocked_ || name_.isEmpty() || !wire::knownEra(era_) || !wire::knownScene(location_)
        || retry_.isActive() || socket_.state() != QAbstractSocket::UnconnectedState) return;
    emit connectionAttempting();
    const auto host = qEnvironmentVariable("ARCANE_SERVER_HOST", "127.0.0.1");
    bool ok = false;
    const auto port = qEnvironmentVariable("ARCANE_SERVER_PORT", "8888").toUShort(&ok);
    handshakeDeadline_.start(5000);
    socket_.connectToHost(host, ok ? port : 8888);
}

void CampusWireClient::retryAfterDisconnect()
{
    ready_ = false;
    handshakeDeadline_.stop();
    if (blocked_ || retry_.isActive()) return;
    retry_.start(retryDelayMs_);
    retryDelayMs_ = std::min(8000, retryDelayMs_ * 2);
    if (!offlineReported_) {
        offlineReported_ = true;
        report(QStringLiteral("校园暂时离线，正在尝试重连；你仍可继续单人探索。"));
    }
}

void CampusWireClient::resetConnection()
{
    const QSignalBlocker blocker(&socket_);
    retry_.stop();
    handshakeDeadline_.stop();
    socket_.abort();
    ready_ = blocked_ = offlineReported_ = false;
    retryDelayMs_ = 1000;
}

void CampusWireClient::receive()
{
    QJsonObject object;
    while (socket_.state() == QAbstractSocket::ConnectedState) {
        switch (wire::readFrame(socket_, object)) {
        case wire::ReadResult::Incomplete: return;
        case wire::ReadResult::TooLarge: socket_.abort(); return;
        case wire::ReadResult::Invalid: continue;
        case wire::ReadResult::Message: break;
        }
        const auto type = object.value("type").toString();
        const auto text = object.value("text").toString();
        if (!ready_) {
            if (type == QLatin1String("error")) {
                const auto code = object.value("code").toString();
                blocked_ = code == QLatin1String("version_mismatch") || code == QLatin1String("invalid_identity");
                // Temporary join failures retry without posting the same notice every time.
                if (blocked_ || !offlineReported_) report(text);
                offlineReported_ = true;
                socket_.abort();
                return;
            }
            if (type != QLatin1String("welcome")) continue;
            if (object.value("version").toInt() != wire::version || object.value("era").toString() != era_) {
                blocked_ = true;
                report(QStringLiteral("联机版本或时代不一致，请更新客户端和服务端。"));
                socket_.abort();
                return;
            }
            ready_ = true;
            offlineReported_ = false;
            retryDelayMs_ = 1000;
            handshakeDeadline_.stop();
            if (helloLocation_ != location_) send({{"type", "move"}, {"location", location_}});
            if (connected()) report(text);
            continue;
        }
        const auto name = object.value("name").toString();
        if (type == QLatin1String("say")) emit messageReceived(QStringLiteral("Location"), name, text);
        else if (type == QLatin1String("owl")) emit messageReceived(QStringLiteral("Owl"), name, text);
        else if (type == QLatin1String("presence")) emit messageReceived(QStringLiteral("World"), name, text);
        else if (type == QLatin1String("error") || type == QLatin1String("owl_sent")) report(text);
    }
}

void CampusWireClient::send(const QJsonObject& object)
{
    if (!wire::writeFrame(socket_, object)) socket_.abort();
}

void CampusWireClient::say(const QString& text)
{
    const auto message = text.trimmed();
    if (connected() && !message.isEmpty() && message.size() <= wire::maxText)
        send({{"type", "say"}, {"text", message}});
}

void CampusWireClient::owl(const QString& target, const QString& text)
{
    const auto recipient = target.trimmed();
    const auto message = text.trimmed();
    if (connected() && !recipient.isEmpty() && recipient.size() <= wire::maxNickname
        && !message.isEmpty() && message.size() <= wire::maxText)
        send({{"type", "owl"}, {"target", recipient}, {"text", message}});
}

bool CampusWireClient::connected() const
{
    return ready_ && socket_.state() == QAbstractSocket::ConnectedState;
}

void CampusWireClient::report(const QString& text)
{
    emit messageReceived(QStringLiteral("System"), QStringLiteral("校园联机"), text);
}
} // namespace arcane::network
