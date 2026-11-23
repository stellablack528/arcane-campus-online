#include "application/open_world/npc/NpcCatalog.hpp"

#include <QJsonArray>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <cmath>
#include <utility>

namespace arcane::application::open_world::npc {
namespace {
using namespace Qt::StringLiterals;

QString requiredText(const QJsonObject& object, const QString& key)
{
    return object.value(key).toString().trimmed();
}

bool readText(const QJsonObject& object, const QString& key, LocalText& text)
{
    const auto value = object.value(key).toObject();
    text = {requiredText(value, u"zh"_s), requiredText(value, u"en"_s)};
    return !text.zh.isEmpty() && !text.en.isEmpty();
}

bool readStrings(const QJsonValue& raw, QStringList& values)
{
    if (!raw.isArray()) return false;
    for (const auto& entry : raw.toArray()) {
        const auto value = entry.toString().trimmed();
        if (value.isEmpty()) return false;
        values.push_back(value);
    }
    return !values.isEmpty();
}

bool readProfile(const QJsonObject& object, Profile& profile)
{
    const auto persona = object.value(u"persona"_s).toObject();
    if (!readText(object, u"name"_s, profile.name)
        || !readStrings(object.value(u"aliases"_s), profile.aliases)
        || !readText(persona, u"traits"_s, profile.persona.traits)
        || !readText(persona, u"speech"_s, profile.persona.speech)
        || !readText(persona, u"boundaries"_s, profile.persona.boundaries)
        || (persona.contains(u"sources"_s) && !readStrings(persona.value(u"sources"_s), profile.persona.sources))
        || !readText(object, u"first_meeting"_s, profile.firstMeeting)
        || !readText(object, u"familiar_greeting"_s, profile.familiarGreeting)
        || !readText(object, u"unknown_topic"_s, profile.unknownTopic)) return false;
    profile.aliases << profile.name.zh << profile.name.en;
    for (auto& alias : profile.aliases) alias = alias.toCaseFolded();
    profile.aliases.removeDuplicates();

    static const QRegularExpression sceneId(u"^[a-z][a-z0-9_]{0,63}$"_s);
    if (object.contains(u"custom_fields"_s)) {
        const auto fields = object.value(u"custom_fields"_s);
        if (!fields.isObject() || fields.toObject().size() > 24) return false;
        profile.customFields = fields.toObject();
        for (auto it = profile.customFields.constBegin(); it != profile.customFields.constEnd(); ++it) {
            if (!sceneId.match(it.key()).hasMatch()) return false;
            const auto value = it.value();
            if (value.isBool()) continue;
            if (value.isString() && !value.toString().trimmed().isEmpty() && value.toString().size() <= 1000) continue;
            if (value.isDouble() && std::abs(value.toDouble()) <= 1'000'000'000
                && std::trunc(value.toDouble()) == value.toDouble()) continue;
            LocalText localized;
            if (value.isObject() && value.toObject().size() == 2 && readText(profile.customFields, it.key(), localized)
                && localized.zh.size() <= 1000 && localized.en.size() <= 1000) continue;
            return false;
        }
    }
    if (object.contains(u"legacy_member_id"_s)) {
        profile.legacyMemberId = requiredText(object, u"legacy_member_id"_s);
        if (!sceneId.match(profile.legacyMemberId).hasMatch()) return false;
    }
    for (const auto& raw : object.value(u"appearances"_s).toArray()) {
        const auto entry = raw.toObject();
        Appearance slot;
        slot.sceneId = requiredText(entry, u"scene"_s);
        if (!sceneId.match(slot.sceneId).hasMatch() || !readText(entry, u"hint"_s, slot.hint)) return false;
        for (const auto& rawPhase : entry.value(u"phases"_s).toArray()) {
            const int phase = rawPhase.toInt(-1);
            if (phase < 0 || phase > 6) return false;
            for (const auto& other : std::as_const(profile.appearances))
                if (other.phases.contains(phase)) return false;
            if (slot.phases.contains(phase)) return false;
            slot.phases.push_back(phase);
        }
        if (slot.phases.isEmpty()) return false;
        profile.appearances.push_back(std::move(slot));
    }
    QSet<QString> topicIds;
    QSet<QString> topicAliases;
    for (const auto& raw : object.value(u"topics"_s).toArray()) {
        const auto entry = raw.toObject();
        Topic topic;
        topic.id = requiredText(entry, u"id"_s);
        if (!sceneId.match(topic.id).hasMatch() || topicIds.contains(topic.id)
            || !readStrings(entry.value(u"aliases"_s), topic.aliases)
            || !readText(entry, u"reply"_s, topic.reply)) return false;
        if (entry.contains(u"repeat_reply"_s) && !readText(entry, u"repeat_reply"_s, topic.repeatReply.emplace()))
            return false;
        topicIds.insert(topic.id);
        for (auto& alias : topic.aliases) {
            alias = alias.toCaseFolded();
            if (topicAliases.contains(alias)) return false;
            topicAliases.insert(alias);
        }
        profile.topics.push_back(std::move(topic));
    }
    return !profile.appearances.isEmpty() && !profile.topics.isEmpty();
}
}

QString Persona::instructions(bool chinese) const
{
    return traits.forLanguage(chinese) + u'\n' + speech.forLanguage(chinese) + u'\n'
        + boundaries.forLanguage(chinese);
}

bool Profile::isPresent(const QString& scene, int phase) const
{
    for (const auto& slot : appearances)
        if (slot.sceneId == scene && slot.phases.contains(phase)) return true;
    return false;
}

QStringList Catalog::loadBuiltIns()
{
    const QDir directory(u":/story/worlds/npcs"_s);
    const auto entries = directory.entryList({u"*.json"_s}, QDir::Files);
    if (entries.isEmpty()) return {u"No built-in NPC packs found."_s};
    QStringList errors;
    for (const auto& entry : entries) {
        QFile file(directory.filePath(entry));
        const auto era = entry.chopped(5);
        QString error;
        if (!file.open(QIODevice::ReadOnly)) error = file.errorString();
        else if (loadPack(file.readAll(), era, error)) continue;
        errors << u"%1: %2"_s.arg(era, error);
    }
    return errors;
}

bool Catalog::loadPack(const QByteArray& json, const QString& era, QString& error)
{
    return load(json, era, false, error);
}

bool Catalog::loadCustomPack(const QByteArray& json, const QString& era, QString& error)
{
    return load(json, era, true, error);
}

bool Catalog::load(const QByteArray& json, const QString& era, bool custom, QString& error)
{
    if (custom && json.size() > 256 * 1024) {
        error = u"Custom NPC packs are limited to 256 KiB."_s;
        return false;
    }
    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson(json, &parseError);
    const auto root = doc.object();
    const auto entries = root.value(u"profiles"_s).toArray();
    if (parseError.error != QJsonParseError::NoError || root.value(u"format"_s).toString() != u"campus-npc/v1"_s
        || root.value(u"era"_s).toString() != era || entries.isEmpty() || (custom && entries.size() > 32)) {
        error = u"Invalid NPC pack header: %1"_s.arg(era);
        return false;
    }
    QList<Profile> staged;
    QSet<QString> ids;
    QSet<QString> aliases;
    for (const auto& existing : std::as_const(profiles_)) {
        ids.insert(existing.id);
        if (existing.era == era)
            for (const auto& alias : existing.aliases) aliases.insert(alias);
    }
    static const QRegularExpression npcId(u"^[a-z][a-z0-9_]*\\.[a-z][a-z0-9_]*$"_s);
    static const QSet<QString> campusScenes{u"great_hall"_s, u"library"_s, u"courtyard"_s, u"charms_classroom"_s,
        u"transfiguration_classroom"_s, u"greenhouse_three"_s, u"defense_classroom"_s, u"hogsmeade"_s};
    for (const auto& raw : entries) {
        const auto entry = raw.toObject();
        Profile profile;
        profile.era = era;
        profile.id = requiredText(entry, u"id"_s);
        if (profile.id.size() > 60 || !npcId.match(profile.id).hasMatch()
            || !profile.id.startsWith(era + u'.') || ids.contains(profile.id)
            || (custom && (!profile.id.startsWith(era + u".custom_"_s) || profile.id.endsWith(u".custom_"_s)
                           || entry.contains(u"legacy_member_id"_s)))
            || !readProfile(entry, profile)) {
            error = u"Invalid or duplicate NPC profile: %1 / %2"_s.arg(era, profile.id);
            return false;
        }
        if (custom) for (const auto& slot : profile.appearances) {
            if (campusScenes.contains(slot.sceneId)) continue;
            error = u"Unreachable campus scene for %1: %2"_s.arg(profile.id, slot.sceneId);
            return false;
        }
        for (const auto& alias : std::as_const(profile.aliases)) {
            if (aliases.contains(alias)) {
                error = u"Ambiguous NPC alias in %1: %2"_s.arg(era, alias);
                return false;
            }
            aliases.insert(alias);
        }
        ids.insert(profile.id);
        staged.push_back(std::move(profile));
    }
    profiles_.append(staged);
    error.clear();
    return true;
}

const Profile* Catalog::find(const QString& id) const
{
    for (const auto& profile : profiles_)
        if (profile.id == id) return &profile;
    return nullptr;
}

std::optional<TalkRequest> Catalog::matchInput(const QString& raw, const QString& era) const
{
    auto text = raw.trimmed().toCaseFolded();
    while (!text.isEmpty() && QStringView(u"。！？?!").contains(text.back())) text.chop(1);
    if (text.startsWith(u"我想")) text = text.mid(2).trimmed();
    const bool explicitNpc = text == u"/npc"_s || text.startsWith(u"/npc "_s);
    static const QStringList prefixes{u"和"_s, u"跟"_s, u"与"_s, u"向"_s, u"找"_s, u"talk to "_s};
    static const QStringList suffixes{u"聊聊"_s, u"聊"_s, u"谈谈"_s, u"谈"_s, u"打招呼"_s,
                                      u"问问"_s, u"问"_s, u"请教"_s, u"说"_s, u" about "_s};
    QString target;
    if (explicitNpc) target = text.mid(4).trimmed();
    else {
        for (const auto& prefix : prefixes)
            if (text.startsWith(prefix)) { target = text.mid(prefix.size()).trimmed(); break; }
    }
    if (target.isEmpty()) return explicitNpc ? std::optional<TalkRequest>{TalkRequest{}} : std::nullopt;

    const Profile* best = nullptr;
    qsizetype bestLength = 0;
    QString topic;
    for (const auto& profile : profiles_) {
        QStringList names = profile.aliases;
        if (explicitNpc) names << profile.id;
        for (const auto& name : names) {
            if (!target.startsWith(name) || name.size() < bestLength) continue;
            QString rest = target.mid(name.size());
            if (rest.startsWith(u':') || rest.startsWith(u'：')) rest.remove(0, 1);
            else if (!rest.trimmed().isEmpty()) {
                bool validSuffix = false;
                for (const auto& suffix : suffixes)
                    if (rest.trimmed().startsWith(suffix.trimmed()) &&
                        (suffix != u" about "_s || rest.startsWith(suffix))) {
                        rest = rest.trimmed().mid(suffix.trimmed().size());
                        validSuffix = true;
                        break;
                    }
                if (!validSuffix) continue;
            }
            if (name.size() == bestLength && best && best->era == era) continue;
            best = &profile;
            bestLength = name.size();
            topic = rest.trimmed();
        }
    }
    if (best) return TalkRequest{best->id, topic};
    if (explicitNpc) return TalkRequest{};
    return std::nullopt;
}

} // namespace arcane::application::open_world::npc
