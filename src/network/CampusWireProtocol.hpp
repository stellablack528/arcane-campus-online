#pragma once

#include <QJsonObject>
#include <QString>

class QTcpSocket;

namespace arcane::network::wire {

inline constexpr int version = 2;
inline constexpr qsizetype maxFrameBytes = 4096;
inline constexpr qsizetype readBufferBytes = 4 * (maxFrameBytes + 1);
inline constexpr qsizetype maxQueuedBytes = 64 * 1024;
inline constexpr qsizetype maxNickname = 32;
inline constexpr qsizetype maxText = 500;

struct Room {
    QString era;
    QString scene;
    bool operator==(const Room&) const = default;
};

bool knownEra(const QString& era);
bool knownScene(const QString& scene);
QString canonicalScene(const QString& location);

enum class ReadResult { Incomplete, Message, Invalid, TooLarge };

// TCP preserves byte order, not message boundaries. A frame ends at LF.
ReadResult readFrame(QTcpSocket& socket, QJsonObject& message);
bool writeFrame(QTcpSocket& socket, const QJsonObject& message);

} // namespace arcane::network::wire
