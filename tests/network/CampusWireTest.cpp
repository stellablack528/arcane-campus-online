#include "network/CampusWireClient.hpp"
#include "network/CampusWireServer.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QHostAddress>
#include <QThread>

#include <iostream>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    arcane::network::CampusWireServer server;
    if (!server.listen(QHostAddress(QStringLiteral("127.0.0.1")), 0)) return 1;
    qputenv("ARCANE_SERVER_PORT", QByteArray::number(server.port()));
    qputenv("ARCANE_SERVER_HOST", "127.0.0.1");
    QStringList aliceMessages, bobMessages;
    arcane::network::CampusWireClient alice, bob;
    QObject::connect(&alice, &arcane::network::CampusWireClient::messageReceived,
                     [&](const QString&, const QString&, const QString& text) { aliceMessages << text; });
    QObject::connect(&bob, &arcane::network::CampusWireClient::messageReceived,
                     [&](const QString&, const QString&, const QString& text) { bobMessages << text; });
    auto settle = [&] {
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < 150) {
            app.processEvents(QEventLoop::AllEvents, 20);
            QThread::msleep(2);
        }
    };
    alice.join(QStringLiteral("Alice"), QStringLiteral("great_hall"));
    bob.join(QStringLiteral("Bob"), QStringLiteral("great_hall"));
    settle();
    if (!alice.connected() || !bob.connected()) {
        std::cerr << "connection failed port=" << server.port() << " Alice="
                  << aliceMessages.join("|").toStdString() << " Bob="
                  << bobMessages.join("|").toStdString() << '\n';
        return 2;
    }
    alice.say(QStringLiteral("hall hello"));
    settle();
    if (!bobMessages.contains(QStringLiteral("hall hello"))) return 3;
    bob.move(QStringLiteral("library"));
    settle();
    alice.say(QStringLiteral("private hall"));
    settle();
    if (bobMessages.contains(QStringLiteral("private hall"))) return 4;
    alice.owl(QStringLiteral("Bob"), QStringLiteral("owl arrived"));
    settle();
    if (!bobMessages.contains(QStringLiteral("owl arrived"))) return 5;
    std::cout << "PASS: shared room chat, scene isolation, cross-scene owl\n";
    return 0;
}
