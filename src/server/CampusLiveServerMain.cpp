#include "network/CampusWireServer.hpp"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTextStream>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    arcane::network::CampusWireServer server;
    const auto address = qEnvironmentVariable("ARCANE_SERVER_BIND", "127.0.0.1");
    bool ok = false;
    const auto port = qEnvironmentVariable("ARCANE_SERVER_PORT", "8888").toUShort(&ok);
    if (!server.listen(QHostAddress(address), ok ? port : 8888)) return 1;
    QTextStream(stdout) << "Campus live server listening on " << address << ':' << server.port() << Qt::endl;
    return app.exec();
}
