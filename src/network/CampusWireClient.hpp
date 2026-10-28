#pragma once

#include <QObject>
#include <QJsonObject>
#include <QTcpSocket>

namespace arcane::network {

class CampusWireClient final : public QObject {
    Q_OBJECT
public:
    explicit CampusWireClient(QObject* parent = nullptr);
    void join(const QString& name, const QString& location);
    void move(const QString& location);
    void say(const QString& text);
    void owl(const QString& target, const QString& text);
    bool connected() const;

signals:
    void messageReceived(const QString& channel, const QString& speaker, const QString& text);

private:
    void send(const QJsonObject& object);
    QTcpSocket socket_;
    QByteArray pending_;
    QString name_;
    QString location_;
};
} // namespace arcane::network
