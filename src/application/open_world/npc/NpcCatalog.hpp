#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QStringList>
#include <optional>

namespace arcane::application::open_world::npc {

struct LocalText {
    QString zh;
    QString en;
    [[nodiscard]] const QString& forLanguage(bool chinese) const { return chinese ? zh : en; }
};

struct Appearance {
    QString sceneId;
    QList<int> phases;
    LocalText hint;
};

struct Persona {
    LocalText traits;
    LocalText speech;
    LocalText boundaries;
    QStringList sources; // Optional provenance; never fetched by the runtime.
    [[nodiscard]] QString instructions(bool chinese) const;
};

struct Topic {
    QString id;
    QStringList aliases;
    LocalText reply;
    std::optional<LocalText> repeatReply;
};

struct Profile {
    QString id;
    QString era;
    QString legacyMemberId;
    QStringList aliases;
    LocalText name;
    Persona persona;
    QList<Appearance> appearances;
    LocalText firstMeeting;
    LocalText familiarGreeting;
    LocalText unknownTopic;
    QList<Topic> topics;
    QJsonObject customFields; // Creator data only; never executes or changes state.

    [[nodiscard]] bool isPresent(const QString& scene, int phase) const;
};

struct TalkRequest {
    QString npcId;
    QString topic;
};

class Catalog final {
public:
    [[nodiscard]] QStringList loadBuiltIns(); // Valid packs are kept; returns errors for missing or rejected packs.
    [[nodiscard]] bool loadPack(const QByteArray& json, const QString& era, QString& error);
    [[nodiscard]] bool loadCustomPack(const QByteArray& json, const QString& era, QString& error);
    [[nodiscard]] const Profile* find(const QString& id) const;
    [[nodiscard]] std::optional<TalkRequest> matchInput(const QString& text, const QString& era) const;
    [[nodiscard]] const QList<Profile>& profiles() const { return profiles_; }

private:
    [[nodiscard]] bool load(const QByteArray& json, const QString& era, bool custom, QString& error);
    QList<Profile> profiles_;
};

} // namespace arcane::application::open_world::npc
