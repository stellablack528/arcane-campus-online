#include "network/CampusWireProtocol.hpp"

#include <QJsonDocument>
#include <QTcpSocket>

namespace arcane::network::wire {
namespace {
struct SceneAlias { const char* id; const char* label; };
constexpr SceneAlias scenes[] = {
    {"charms_classroom", "Charms Classroom"}, {"great_hall", "Great Hall"},
    {"library", "Library"}, {"courtyard", "Courtyard"}, {"house_tables", "House Tables"},
    {"transfiguration_classroom", "Transfiguration Classroom"}, {"hogsmeade", "Hogsmeade"},
    {"greenhouse_three", "Greenhouse Three"}, {"defense_classroom", "Defense Classroom"},
    {"forbidden_forest", "Forbidden Forest"},
};
}

bool knownEra(const QString& era)
{
    return era == QLatin1String("parents") || era == QLatin1String("harry")
        || era == QLatin1String("riddle");
}

bool knownScene(const QString& scene)
{
    for (const auto& entry : scenes)
        if (scene == QLatin1String(entry.id)) return true;
    return false;
}

QString canonicalScene(const QString& location)
{
    for (const auto& entry : scenes)
        if (location == QLatin1String(entry.label)) return QString::fromLatin1(entry.id);
    return location;
}

ReadResult readFrame(QTcpSocket& socket, QJsonObject& message)
{
    if (!socket.canReadLine())
        return socket.bytesAvailable() > maxFrameBytes ? ReadResult::TooLarge : ReadResult::Incomplete;

    auto line = socket.readLine(maxFrameBytes + 2);
    if (!line.endsWith('\n') || line.size() > maxFrameBytes + 1) return ReadResult::TooLarge;
    line.chop(1);
    const auto document = QJsonDocument::fromJson(line);
    if (!document.isObject()) return ReadResult::Invalid;
    message = document.object();
    return ReadResult::Message;
}

bool writeFrame(QTcpSocket& socket, const QJsonObject& message)
{
    if (socket.state() != QAbstractSocket::ConnectedState) return false;
    auto frame = QJsonDocument(message).toJson(QJsonDocument::Compact);
    if (frame.size() > maxFrameBytes) return false;
    frame += '\n';
    // QTcpSocket already queues writes in order. Bound that queue instead of adding a second one.
    return socket.bytesToWrite() + frame.size() <= maxQueuedBytes
        && socket.write(frame) == frame.size();
}

} // namespace arcane::network::wire
