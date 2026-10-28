#pragma once

#include <QHash>
#include <QHostAddress>
#include <QJsonObject>
#include <QObject>
#include <QTcpServer>

class QTcpSocket;

namespace arcane::network {

// Small room server for the playable local/LAN prototype. Each TCP frame is
// one compact JSON object followed by LF, capped before parsing.
class CampusWireServer final : public QObject {
    Q_OBJECT
public:
    explicit CampusWireServer(QObject* parent = nullptr);
    ~CampusWireServer() override;
    bool listen(const QHostAddress& address, quint16 port);
    quint16 port() const;

private:
    struct Peer {
        QString name;
        QString location;
        QByteArray pending;
    };
    void receive(QTcpSocket* socket);
    void send(QTcpSocket* socket, const QJsonObject& event);
    void broadcast(const QString& location, const QJsonObject& event, QTcpSocket* except = nullptr);
    QTcpServer server_;
    QHash<QTcpSocket*, Peer> peers_;
};

} // namespace arcane::network
