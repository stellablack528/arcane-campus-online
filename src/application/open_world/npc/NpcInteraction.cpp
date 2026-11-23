#include "application/open_world/npc/NpcInteraction.hpp"

#include "DBConnection.hpp"
#include "DBTransaction.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

namespace arcane::application::open_world::npc {

Interaction::Interaction(std::shared_ptr<database::DBConnection> connection)
    : connection_(std::move(connection)) {}

TalkResult Interaction::talk(const Catalog& catalog, const TalkContext& context,
                             const TalkRequest& request) const
{
    const auto local = [&](const QString& zh, const QString& en) { return context.chinese ? zh : en; };
    const auto* profile = catalog.find(request.npcId);
    if (!profile) return {false, {}, local(QStringLiteral("用法：/npc 名字 或 /npc 名字：话题。"),
                                         QStringLiteral("Try /npc name or /npc name: topic."))};
    const auto& name = profile->name.forLanguage(context.chinese);
    if (profile->era != context.era)
        return {false, {}, local(QStringLiteral("%1不在这个时代。"),
                                QStringLiteral("%1 is not present in this era.")).arg(name)};
    if (!profile->isPresent(context.scene, context.phase)) {
        QStringList hints;
        for (const auto& slot : profile->appearances) hints << slot.hint.forLanguage(context.chinese);
        return {false, {}, local(QStringLiteral("%1现在不在这里。%2"),
                                QStringLiteral("%1 isn't here now. %2")).arg(name, hints.join(QLatin1Char(' ')))};
    }

    const Topic* topic = nullptr;
    const auto key = request.topic.trimmed().toCaseFolded();
    if (!key.isEmpty()) {
        for (const auto& entry : profile->topics)
            if (entry.id == key || entry.aliases.contains(key)) { topic = &entry; break; }
    }
    // This owns only the namespaced open-world row. Chapter characters keep their original IDs.
    const TalkResult saveFailed{false, {}, local(QStringLiteral("这次交谈没能保存，请稍后再试。"),
                                                 QStringLiteral("This conversation could not be saved. Please try again."))};
    if (!connection_ || !context.characterId) return saveFailed;
    database::DBTransaction transaction(connection_);
    if (!transaction.isActive()) return saveFailed;
    const database::QueryParams id{std::to_string(context.characterId), profile->id.toStdString()};
    const auto row = connection_->query(
        "SELECT memory_json FROM npc_memories WHERE character_id = ? AND npc_id = ? FOR UPDATE", id);
    if (!row) return saveFailed;
    QJsonObject memory;
    if (!row->rows.empty()) {
        const auto stored = row->rows.front().find("memory_json");
        if (stored == row->rows.front().end()) return saveFailed;
        const auto parsed = QJsonDocument::fromJson(QByteArray::fromStdString(stored->second));
        if (!parsed.isObject() || !parsed.object().value(QStringLiteral("facts")).isArray()) return saveFailed;
        memory = parsed.object();
    }
    auto facts = memory.value(QStringLiteral("facts")).toArray();
    const auto met = QStringLiteral("met");
    const bool first = !facts.contains(QJsonValue(met));
    const QString topicFact = topic ? QStringLiteral("discussed_%1").arg(topic->id) : QString{};
    const bool newTopic = !topicFact.isEmpty() && !facts.contains(QJsonValue(topicFact));
    if (first) facts.append(met);
    if (newTopic) facts.append(topicFact);
    if (first || newTopic) {
        QStringList flags;
        for (const auto& fact : facts) flags << fact.toString();
        const auto legacy = flags.join(QLatin1Char(';'));
        memory.insert(QStringLiteral("facts"), facts);
        memory.insert(QStringLiteral("legacy_flags"), legacy);
        memory.insert(QStringLiteral("last_scene_id"), context.scene);
        if (first && !memory.contains(QStringLiteral("times_met")))
            memory.insert(QStringLiteral("times_met"), 1); // do not count repeated greetings
        const auto json = QJsonDocument(memory).toJson(QJsonDocument::Compact).toStdString();
        if (!connection_->execute(
                "INSERT INTO npc_memories (character_id, npc_id, affinity, memory_flags, memory_json, revision) "
                "VALUES (?, ?, 0, ?, ?, 1) ON DUPLICATE KEY UPDATE memory_flags = VALUES(memory_flags), "
                "memory_json = VALUES(memory_json), revision = npc_memories.revision + 1",
                {id[0], id[1], legacy.toStdString(), json})) return saveFailed;
    }
    if (!transaction.commit()) return saveFailed;

    QString text;
    if (first || key.isEmpty())
        text = (first ? profile->firstMeeting : profile->familiarGreeting).forLanguage(context.chinese);
    if (!key.isEmpty()) {
        if (!text.isEmpty()) text += QLatin1Char('\n');
        const auto& reply = !topic ? profile->unknownTopic
            : !newTopic && topic->repeatReply ? *topic->repeatReply : topic->reply;
        text += reply.forLanguage(context.chinese);
    }
    return {true, name, text};
}

} // namespace arcane::application::open_world::npc
