#include "network/CampusWireClient.hpp"
#include "network/CampusWireProtocol.hpp"
#include "network/CampusWireServer.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>

#include <iostream>
#include <memory>

namespace {
using arcane::network::CampusWireClient;
using arcane::network::CampusWireServer;
namespace wire = arcane::network::wire;

template<class Predicate>
bool waitFor(Predicate predicate, int timeoutMs = 3000)
{
    QElapsedTimer timer;
    timer.start();
    do {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        if (predicate()) return true;
        QThread::msleep(2);
    } while (timer.elapsed() < timeoutMs);
    return false;
}

struct Player {
    QStringList messages;
    QStringList owls;
    CampusWireClient client;

    Player(const QString& name, const QString& era, const QString& scene)
    {
        QObject::connect(&client, &CampusWireClient::messageReceived, &client,
                         [this](const QString& channel, const QString&, const QString& text) {
            messages.append(text);
            if (channel == QLatin1String("Owl")) owls.append(text);
        });
        client.setEra(era);
        client.join(name, scene);
    }
};

bool check(bool result, const char* scenario)
{
    std::cout << (result ? "PASS: " : "FAIL: ") << scenario << std::endl;
    return result;
}
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    auto server = std::make_unique<CampusWireServer>();
    const QHostAddress loopback(QStringLiteral("127.0.0.1"));
    if (!check(server->listen(loopback, 0), "server listens on a free loopback port")) return 1;
    qputenv("ARCANE_SERVER_PORT", QByteArray::number(server->port()));
    qputenv("ARCANE_SERVER_HOST", "127.0.0.1");

    Player alice("Alice", "harry", "Great Hall"), bob("Bob", "harry", "great_hall");
    Player otherBob("Bob", "parents", "great_hall"), tom("Tom", "riddle", "great_hall");
    if (!check(waitFor([&] { return alice.client.connected() && bob.client.connected()
                                    && otherBob.client.connected() && tom.client.connected(); }),
               "all three eras join; the same nickname may exist in different eras")) return 1;

    alice.client.say("hall-chat");
    if (!check(waitFor([&] { return bob.messages.contains("hall-chat"); })
               && !waitFor([&] { return otherBob.messages.contains("hall-chat") || tom.messages.contains("hall-chat"); }, 100),
               "same scene receives chat only inside the sender's era")) return 1;

    bob.client.move("Library");
    if (!check(waitFor([&] { return alice.messages.contains(QStringLiteral("离开了当前场景。")); }),
               "moving announces departure to the old room")) return 1;
    alice.client.say("hall-only");
    alice.client.owl("Bob", "owl-to-bob");
    if (!check(waitFor([&] { return bob.owls.contains("owl-to-bob"); }) && !bob.messages.contains("hall-only")
               && !waitFor([&] { return otherBob.owls.contains("owl-to-bob"); }, 100),
               "a move stops old room chat; owl crosses scenes but never eras")) return 1;
    alice.client.owl("Tom", "must-not-escape");
    if (!check(waitFor([&] { return alice.messages.contains(QStringLiteral("本时代没有找到可收信的在线角色。")); })
               && !tom.owls.contains("must-not-escape"), "an owl to another era reports not found")) return 1;

    {
        Player duplicate("Bob", "harry", "great_hall");
        if (!check(waitFor([&] { return duplicate.messages.contains(QStringLiteral("本时代已有同名角色在线。")); })
                   && !duplicate.client.connected(), "a TCP connection alone does not mean the server accepted the player")) return 1;
        // Close this client before restarting the lobby. Nicknames are not reserved across restarts.
    }

    QTcpSocket raw;
    raw.connectToHost(loopback, server->port());
    if (!waitFor([&] { return raw.state() == QAbstractSocket::ConnectedState; })) return 1;
    const QByteArray hello = QByteArray("{\"type\":\"hello\",\"version\":") + QByteArray::number(wire::version)
        + ",\"name\":\"Raw\",\"era\":\"harry\",\"location\":\"great_hall\"}";
    raw.write(hello.first(10));
    if (!check(!waitFor([&] { return raw.canReadLine(); }, 100), "a partial frame is not processed")) return 1;
    raw.write(hello.sliced(10) + "\nnot json\n{\"type\":\"say\",\"text\":\"first\"}\n"
                                "{\"type\":\"say\",\"text\":\"second\"}\n");
    if (!check(waitFor([&] { return alice.messages.contains("second"); })
               && alice.messages.indexOf("first") >= 0 && alice.messages.indexOf("first") < alice.messages.indexOf("second"),
               "split and coalesced frames keep order and malformed JSON is skipped")) return 1;
    const QByteArray limit = QByteArray("{\"type\":\"say\",\"text\":\"max-frame\"}").leftJustified(wire::maxFrameBytes, ' ');
    raw.write(limit + '\n');
    if (!check(waitFor([&] { return alice.messages.contains("max-frame"); }), "the exact frame limit is accepted")) return 1;
    raw.write(QByteArray(wire::maxFrameBytes + 1, ' '));
    if (!check(waitFor([&] { return raw.state() == QAbstractSocket::UnconnectedState; }),
               "an oversized frame without a newline is disconnected")) return 1;

    QTcpSocket oldClient;
    QByteArray legacyReply;
    QObject::connect(&oldClient, &QTcpSocket::readyRead, &oldClient, [&] { legacyReply += oldClient.readAll(); });
    oldClient.connectToHost(loopback, server->port());
    if (!waitFor([&] { return oldClient.state() == QAbstractSocket::ConnectedState; })) return 1;
    oldClient.write("{\"type\":\"hello\",\"name\":\"Legacy\",\"location\":\"great_hall\"}\n");
    if (!check(waitFor([&] { return oldClient.state() == QAbstractSocket::UnconnectedState; })
               && legacyReply.contains("version_mismatch") && !legacyReply.contains("welcome"),
               "a legacy client receives a version error instead of joining an arbitrary era")) return 1;

    const auto originalPort = server->port();
    server.reset();
    if (!check(waitFor([&] { return !alice.client.connected() && !bob.client.connected() && !otherBob.client.connected(); }),
               "clients notice the connection loss")) return 1;
    alice.client.move("Courtyard");
    bob.client.move("courtyard");
    otherBob.client.move("courtyard");
    server = std::make_unique<CampusWireServer>();
    if (!check(server->listen(loopback, originalPort), "the restarted lobby listens on its original port")) return 1;
    if (!check(waitFor([&] { return alice.client.connected() && bob.client.connected() && otherBob.client.connected(); }, 5000),
               "clients reconnect without restarting the game")) return 1;
    alice.client.say("after-reconnect");
    if (!check(waitFor([&] { return bob.messages.contains("after-reconnect"); })
               && !waitFor([&] { return otherBob.messages.contains("after-reconnect"); }, 100),
               "reconnect uses the latest offline scene and preserves era isolation")) return 1;
    return 0;
}
