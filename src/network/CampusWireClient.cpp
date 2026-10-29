#include "network/CampusWireClient.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>

namespace arcane::network {
CampusWireClient::CampusWireClient(QObject* parent) : QObject(parent)
{
    connect(&socket_, &QTcpSocket::connected, this, [this] {
        send({{"type", "hello"}, {"name", name_}, {"location", location_}});
    });
    connect(&socket_, &QTcpSocket::readyRead, this, [this] {
        pending_ += socket_.readAll();
        while (true) {
            const auto end = pending_.indexOf('\n');
            if (end < 0) break;
            const auto frame = pending_.left(end);
            pending_.remove(0, end + 1);
            if (frame.size() > 4096) continue;
            const auto object = QJsonDocument::fromJson(frame).object();
            const auto type = object.value("type").toString();
            if (type == QLatin1String("say"))
                emit messageReceived(QStringLiteral("Location"), object.value("name").toString(), object.value("text").toString());
            else if (type == QLatin1String("owl"))
                emit messageReceived(QStringLiteral("Owl"), object.value("name").toString(), object.value("text").toString());
            else if (type == QLatin1String("presence"))
                emit messageReceived(QStringLiteral("World"), object.value("name").toString(), object.value("text").toString());
            else if (type == QLatin1String("welcome") || type == QLatin1String("error") || type == QLatin1String("owl_sent"))
                emit messageReceived(QStringLiteral("System"), QStringLiteral("校园联机"), object.value("text").toString());
        }
        if (pending_.size() > 4096) socket_.disconnectFromHost();
    });
    connect(&socket_, &QTcpSocket::disconnected, this, [this] {
        if (!name_.isEmpty()) emit messageReceived(QStringLiteral("System"), QStringLiteral("校园联机"), QStringLiteral("连接已断开；当前可继续单人探索。"));
    });
    connect(&socket_, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        emit messageReceived(QStringLiteral("System"), QStringLiteral("校园联机"),
                             QStringLiteral("连接失败：%1").arg(socket_.errorString()));
    });
}

void CampusWireClient::join(const QString& name, const QString& location)
{
    if (socket_.state() != QAbstractSocket::UnconnectedState) return;
    name_ = name;
    location_ = location;
    const auto host = qEnvironmentVariable("ARCANE_SERVER_HOST", "127.0.0.1");
    bool ok = false;
    const auto port = qEnvironmentVariable("ARCANE_SERVER_PORT", "8888").toUShort(&ok);
    socket_.connectToHost(host, ok ? port : 8888);
}
void CampusWireClient::send(const QJsonObject& object)
{
    if (connected()) socket_.write(QJsonDocument(object).toJson(QJsonDocument::Compact) + '\n');
}
void CampusWireClient::move(const QString& location)
{
    QString id = location;
    if (id == QLatin1String("Great Hall")) id = QStringLiteral("great_hall");
    else if (id == QLatin1String("Library")) id = QStringLiteral("library");
    else if (id == QLatin1String("Courtyard")) id = QStringLiteral("courtyard");
    else if (id == QLatin1String("Transfiguration Classroom")) id = QStringLiteral("transfiguration_classroom");
    else if (id == QLatin1String("Hogsmeade")) id = QStringLiteral("hogsmeade");
    if (id == location_) return;
    location_ = id;
    send({{"type", "move"}, {"location", id}});
}
void CampusWireClient::say(const QString& text) { send({{"type", "say"}, {"text", text}}); }
void CampusWireClient::owl(const QString& target, const QString& text)
{
    send({{"type", "owl"}, {"target", target}, {"text", text}});
}
bool CampusWireClient::connected() const { return socket_.state() == QAbstractSocket::ConnectedState; }
} // namespace arcane::network
