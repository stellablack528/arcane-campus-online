#pragma once

#include "network/CampusWireProtocol.hpp"

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
    // The socket owns transport state; Peer is the identity accepted by the server.
    struct Peer {
        QString name;
        wire::Room room;
    };
    void receive(QTcpSocket* socket);
    bool handleRequest(QTcpSocket* socket, Peer& peer, const QJsonObject& request);
    static bool send(QTcpSocket* socket, const QJsonObject& event);
    void broadcast(const wire::Room& room, const QJsonObject& event, QTcpSocket* except);
    void announce(const Peer& peer, bool entering, QTcpSocket* except);
    QTcpServer server_;
    QHash<QTcpSocket*, Peer> peers_;
};

} // namespace arcane::network
