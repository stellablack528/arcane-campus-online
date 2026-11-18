#pragma once

#include <QObject>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTimer>

namespace arcane::network {

class CampusWireClient final : public QObject {
    Q_OBJECT
public:
    explicit CampusWireClient(QObject* parent = nullptr);
    ~CampusWireClient() override;
    void setEra(const QString& era);
    void join(const QString& name, const QString& location);
    void move(const QString& location);
    void say(const QString& text);
    void owl(const QString& target, const QString& text);
    bool connected() const;

signals:
    void connectionAttempting();
    void messageReceived(const QString& channel, const QString& speaker, const QString& text);

private:
    void tryConnect();
    void retryAfterDisconnect();
    void resetConnection();
    void receive();
    void send(const QJsonObject& object);
    void report(const QString& text);
    QTcpSocket socket_;
    QTimer retry_;
    QTimer handshakeDeadline_;
    QString name_;
    QString era_;
    QString location_;
    QString helloLocation_;
    int retryDelayMs_ = 1000;
    bool ready_ = false;
    bool blocked_ = false;
    bool offlineReported_ = false;
};
} // namespace arcane::network
