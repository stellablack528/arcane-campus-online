#include "application/story/persistence/StoryPersistenceService.hpp"

#include "DBConnection.hpp"
#include "DBTransaction.hpp"
#include "RecordMapper.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <limits>
#include <utility>

namespace arcane::application::story::persistence {

namespace {

using database::QueryParams;
using database::QueryRow;

QString databaseError(const std::shared_ptr<database::DBConnection>& connection,
                      const QString& fallback)
{
    if (!connection || connection->lastError().empty()) return fallback;
    return QString::fromStdString(connection->lastError());
}

QStringList sorted(const QSet<QString>& values)
{
    QStringList result(values.begin(), values.end());
    result.sort();
    return result;
}

QJsonArray stringArray(const QSet<QString>& values)
{
    QJsonArray array;
    for (const auto& value : sorted(values)) array.push_back(value);
    return array;
}

QSet<QString> parseStringArray(const QJsonValue& value, bool& valid)
{
    QSet<QString> result;
    if (value.isUndefined() || value.isNull()) return result;
    if (!value.isArray()) {
        valid = false;
        return result;
    }
    for (const auto& entry : value.toArray()) {
        if (!entry.isString() || entry.toString().trimmed().isEmpty()) {
            valid = false;
            return {};
        }
        result.insert(entry.toString());
    }
    return result;
}

QSet<QString> splitLegacyFlags(const std::string& text)
{
    const auto values = QString::fromStdString(text).split(QLatin1Char(';'), Qt::SkipEmptyParts);
    QSet<QString> result;
    for (const auto& value : values) result.insert(value.trimmed());
    return result;
}

QByteArray stateJson(const StoryRuntimeState& state)
{
    QJsonObject object;
    object.insert(QStringLiteral("flags"), stringArray(state.flags));
    object.insert(QStringLiteral("visited_scenes"), stringArray(state.visitedScenes));
    object.insert(QStringLiteral("unlocked_chapters"), stringArray(state.unlockedChapters));
    object.insert(QStringLiteral("chapter_completed"), state.chapterCompleted);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool applyStateJson(const std::string& json,
                    StoryRuntimeState& state,
                    QString& error)
{
    if (json.empty()) return true;
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(QByteArray::fromStdString(json), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        error = QStringLiteral("The saved story state JSON is invalid.");
        return false;
    }
    const auto object = document.object();
    bool valid = true;
    state.flags.unite(parseStringArray(object.value(QStringLiteral("flags")), valid));
    state.visitedScenes.unite(
        parseStringArray(object.value(QStringLiteral("visited_scenes")), valid));
    state.unlockedChapters.unite(
        parseStringArray(object.value(QStringLiteral("unlocked_chapters")), valid));
    if (object.contains(QStringLiteral("chapter_completed"))) {
        if (!object.value(QStringLiteral("chapter_completed")).isBool()) valid = false;
        else state.chapterCompleted = object.value(QStringLiteral("chapter_completed")).toBool();
    }
    if (object.value(QStringLiteral("legacy_flags")).isString()) {
        state.flags.unite(splitLegacyFlags(
            object.value(QStringLiteral("legacy_flags")).toString().toStdString()));
    }
    if (!valid) error = QStringLiteral("The saved story state has an invalid structure.");
    return valid;
}

bool collectUnlockedChapters(const std::string& json,
                             QSet<QString>& destination,
                             QString& error)
{
    if (json.empty()) return true;
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(QByteArray::fromStdString(json), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        error = QStringLiteral("A saved chapter contains invalid state JSON.");
        return false;
    }
    bool valid = true;
    destination.unite(parseStringArray(
        document.object().value(QStringLiteral("unlocked_chapters")), valid));
    if (!valid) error = QStringLiteral("A saved chapter has invalid unlocked chapter data.");
    return valid;
}

QByteArray updatedMemoryJson(const QueryRow* row,
                             const QSet<QString>& facts,
                             const QString& sceneId)
{
    QJsonObject object;
    if (row) {
        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(
            QByteArray::fromStdString(database::detail::value(*row, "memory_json")),
            &parseError);
        if (parseError.error == QJsonParseError::NoError && document.isObject())
            object = document.object();
    }
    const QString legacyFlags = sorted(facts).join(QLatin1Char(';'));
    object.insert(QStringLiteral("legacy_flags"), legacyFlags);
    object.insert(QStringLiteral("facts"), stringArray(facts));
    object.insert(QStringLiteral("last_scene_id"), sceneId);
    if (!object.contains(QStringLiteral("times_met")))
        object.insert(QStringLiteral("times_met"), 0);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool memoryFacts(const QueryRow& row, QSet<QString>& facts, QString& error)
{
    const auto json = database::detail::value(row, "memory_json");
    if (json.empty()) {
        facts = splitLegacyFlags(database::detail::value(row, "memory_flags"));
        return true;
    }
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(QByteArray::fromStdString(json), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        error = QStringLiteral("An NPC memory record contains invalid JSON.");
        return false;
    }
    bool valid = true;
    facts = parseStringArray(document.object().value(QStringLiteral("facts")), valid);
    if (facts.isEmpty()
        && document.object().value(QStringLiteral("legacy_flags")).isString()) {
        facts = splitLegacyFlags(document.object()
                                     .value(QStringLiteral("legacy_flags"))
                                     .toString()
                                     .toStdString());
    }
    if (!valid) error = QStringLiteral("An NPC memory record has an invalid structure.");
    return valid;
}

QByteArray eventJson(const StoryExecutionResult& result,
                     const StoryRuntimeState& state,
                     std::uint64_t nextRevision)
{
    const auto& delta = result.delta;
    QJsonObject object;
    object.insert(QStringLiteral("success"), result.success);
    object.insert(QStringLiteral("previous_revision"), QString::number(state.revision));
    object.insert(QStringLiteral("revision"), QString::number(nextRevision));
    object.insert(QStringLiteral("flags_added"), stringArray(delta.flagsToAdd));
    object.insert(QStringLiteral("flags_removed"), stringArray(delta.flagsToRemove));
    object.insert(QStringLiteral("gold_change"), QString::number(delta.goldChange));
    object.insert(QStringLiteral("chapter_completed"), delta.completeChapter);
    if (delta.nextSceneId) object.insert(QStringLiteral("next_scene"), *delta.nextSceneId);
    if (delta.nextChapterId) object.insert(QStringLiteral("next_chapter"), *delta.nextChapterId);

    QJsonArray inventory;
    for (const auto& change : delta.inventoryChanges) {
        inventory.push_back(QJsonObject{
            {QStringLiteral("item_id"), QString::number(change.itemId)},
            {QStringLiteral("quantity"), change.quantity},
        });
    }
    object.insert(QStringLiteral("inventory_changes"), inventory);

    QJsonArray affinities;
    for (const auto& change : delta.affinityChanges) {
        affinities.push_back(QJsonObject{
            {QStringLiteral("npc_id"), change.npcId},
            {QStringLiteral("amount"), change.amount},
        });
    }
    object.insert(QStringLiteral("affinity_changes"), affinities);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool sameInventoryQuantity(const QueryRow* row, std::uint32_t expected)
{
    const std::uint32_t actual = row
        ? database::detail::integer<std::uint32_t>(*row, "quantity")
        : 0;
    return actual == expected;
}

} // namespace

StoryPersistenceService::StoryPersistenceService(
    std::shared_ptr<database::DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

StoryLoadResult StoryPersistenceService::load(const ChapterDefinition& chapter,
                                              std::uint64_t characterId) const
{
    StoryLoadResult loaded;
    if (!connection_ || !connection_->isConnected()) {
        loaded.error = QStringLiteral("Story storage is unavailable.");
        return loaded;
    }
    if (characterId == 0 || chapter.chapterId.isEmpty() || chapter.contentVersion <= 0) {
        loaded.error = QStringLiteral("The story restore request is invalid.");
        return loaded;
    }

    database::DBTransaction transaction(connection_);
    if (!transaction.isActive()) {
        loaded.error = databaseError(connection_, QStringLiteral("Could not start story restore."));
        return loaded;
    }

    StoryRuntimeState state;
    state.characterId = characterId;
    state.chapterId = chapter.chapterId;
    state.contentVersion = chapter.contentVersion;
    state.currentSceneId = chapter.entrySceneId;
    const QueryParams characterParams{std::to_string(characterId)};
    const auto character = connection_->query(
        "SELECT character_id, gold, house, blood_status FROM characters WHERE character_id = ?",
        characterParams);
    if (!character) {
        loaded.error = databaseError(connection_, QStringLiteral("The character could not be loaded."));
        return loaded;
    }
    if (character->rows.empty()) {
        loaded.error = QStringLiteral("The character does not exist.");
        return loaded;
    }
    const auto& characterRow = character->rows.front();
    state.gold = database::detail::integer<std::uint64_t>(characterRow, "gold");
    state.characterAttributes.insert(
        QStringLiteral("house"),
        QString::fromStdString(database::detail::value(characterRow, "house")));
    state.characterAttributes.insert(
        QStringLiteral("blood_status"),
        QString::fromStdString(database::detail::value(characterRow, "blood_status")));

    const auto inventory = connection_->query(
        "SELECT item_id, quantity FROM inventories WHERE character_id = ?",
        characterParams);
    if (!inventory) {
        loaded.error = databaseError(connection_, QStringLiteral("The inventory could not be loaded."));
        return loaded;
    }
    for (const auto& row : inventory->rows) {
        const auto itemId = database::detail::integer<std::uint64_t>(row, "item_id");
        const auto quantity = database::detail::integer<std::uint32_t>(row, "quantity");
        if (itemId == 0 || quantity == 0) {
            loaded.error = QStringLiteral("The inventory contains an invalid story item record.");
            return loaded;
        }
        state.inventory.insert(itemId, quantity);
    }

    const auto memories = connection_->query(
        "SELECT npc_id, affinity, memory_flags, memory_json FROM npc_memories "
        "WHERE character_id = ?",
        characterParams);
    if (!memories) {
        loaded.error = databaseError(connection_, QStringLiteral("NPC memories could not be loaded."));
        return loaded;
    }
    for (const auto& row : memories->rows) {
        const QString npcId = QString::fromStdString(database::detail::value(row, "npc_id"));
        if (npcId.isEmpty()) {
            loaded.error = QStringLiteral("An NPC memory record has no NPC id.");
            return loaded;
        }
        state.npcAffinities.insert(npcId, database::detail::integer<int>(row, "affinity"));
        QSet<QString> facts;
        if (!memoryFacts(row, facts, loaded.error)) return loaded;
        state.npcMemoryFacts.insert(npcId, facts);
    }

    const auto completed = connection_->query(
        "SELECT chapter_id, status, state_json FROM character_story_states "
        "WHERE character_id = ?",
        characterParams);
    if (!completed) {
        loaded.error = databaseError(connection_, QStringLiteral("Completed chapters could not be loaded."));
        return loaded;
    }
    for (const auto& row : completed->rows) {
        const QString id = QString::fromStdString(database::detail::value(row, "chapter_id"));
        if (!id.isEmpty() && database::detail::value(row, "status") == "completed")
            state.completedChapters.insert(id);
        if (!collectUnlockedChapters(
                database::detail::value(row, "state_json"),
                state.unlockedChapters,
                loaded.error)) {
            return loaded;
        }
    }

    const auto wand = connection_->query(
        "SELECT wood_id, core_id FROM character_wands WHERE character_id = ?",
        characterParams);
    if (!wand) {
        loaded.error = databaseError(connection_, QStringLiteral("The wand record could not be loaded."));
        return loaded;
    }
    if (!wand->rows.empty()) {
        state.characterAttributes.insert(
            QStringLiteral("wand_wood"),
            QString::fromStdString(database::detail::value(wand->rows.front(), "wood_id")));
        state.characterAttributes.insert(
            QStringLiteral("wand_core"),
            QString::fromStdString(database::detail::value(wand->rows.front(), "core_id")));
    }

    const auto progress = connection_->query(
        "SELECT chapter_id, content_version, scene_id, status, flags, state_json, revision "
        "FROM character_story_states WHERE character_id = ? AND chapter_id = ?",
        {std::to_string(characterId), chapter.chapterId.toStdString()});
    if (!progress) {
        loaded.error = databaseError(connection_, QStringLiteral("Story progress could not be loaded."));
        return loaded;
    }
    if (!progress->rows.empty()) {
        loaded.found = true;
        const auto& row = progress->rows.front();
        const int savedVersion = database::detail::integer<int>(row, "content_version");
        if (savedVersion != chapter.contentVersion) {
            loaded.error = QStringLiteral("Saved story content version %1 does not match version %2.")
                               .arg(savedVersion)
                               .arg(chapter.contentVersion);
            return loaded;
        }
        state.contentVersion = savedVersion;
        state.revision = database::detail::integer<std::uint64_t>(row, "revision");
        state.currentSceneId = QString::fromStdString(database::detail::value(row, "scene_id"));
        state.flags.unite(splitLegacyFlags(database::detail::value(row, "flags")));
        if (!applyStateJson(database::detail::value(row, "state_json"), state, loaded.error))
            return loaded;
        const auto status = database::detail::value(row, "status");
        if (status == "completed") {
            state.chapterCompleted = true;
            state.completedChapters.insert(chapter.chapterId);
        }
        if (!chapter.findScene(state.currentSceneId)) {
            loaded.error = QStringLiteral("The saved story scene no longer exists.");
            return loaded;
        }
    }

    if (!transaction.commit()) {
        loaded.error = databaseError(connection_, QStringLiteral("Could not finish story restore."));
        return loaded;
    }
    loaded.state = std::move(state);
    loaded.success = true;
    return loaded;
}

bool StoryPersistenceService::commit(const ChapterDefinition& chapter,
                                     const StoryRuntimeState& currentState,
                                     const StoryExecutionResult& result,
                                     QString& error) const
{
    error.clear();
    if (!connection_ || !connection_->isConnected()) {
        error = QStringLiteral("Story storage is unavailable.");
        return false;
    }
    if (!result.success || currentState.characterId == 0
        || currentState.chapterId != chapter.chapterId
        || currentState.contentVersion != chapter.contentVersion) {
        error = QStringLiteral("The story commit request is invalid.");
        return false;
    }
    if (currentState.revision == std::numeric_limits<std::uint64_t>::max()) {
        error = QStringLiteral("The story revision limit has been reached.");
        return false;
    }

    StoryRuntimeState candidate = currentState;
    StoryEngine engine;
    if (!engine.applyDelta(candidate, result.delta, &error)) return false;
    candidate.contentVersion = chapter.contentVersion;
    candidate.revision = currentState.revision + 1;

    database::DBTransaction transaction(connection_);
    if (!transaction.isActive()) {
        error = databaseError(connection_, QStringLiteral("Could not start the story transaction."));
        return false;
    }
    const std::string characterId = std::to_string(currentState.characterId);
    const std::string chapterId = chapter.chapterId.toStdString();

    const auto character = connection_->query(
        "SELECT gold FROM characters WHERE character_id = ? FOR UPDATE", {characterId});
    if (!character || character->rows.empty()) {
        error = databaseError(connection_, QStringLiteral("The character could not be locked."));
        return false;
    }
    if (database::detail::integer<std::uint64_t>(character->rows.front(), "gold")
        != currentState.gold) {
        error = QStringLiteral("The character gold changed in another session. Reload the story.");
        return false;
    }

    const auto progress = connection_->query(
        "SELECT content_version, revision FROM character_story_states "
        "WHERE character_id = ? AND chapter_id = ? FOR UPDATE",
        {characterId, chapterId});
    if (!progress) {
        error = databaseError(connection_, QStringLiteral("Story progress could not be locked."));
        return false;
    }
    const bool hasProgress = !progress->rows.empty();
    if (hasProgress) {
        const auto& row = progress->rows.front();
        if (database::detail::integer<int>(row, "content_version") != chapter.contentVersion
            || database::detail::integer<std::uint64_t>(row, "revision")
                   != currentState.revision) {
            error = QStringLiteral("Story progress changed in another session. Reload the story.");
            return false;
        }
    } else if (currentState.revision != 0) {
        error = QStringLiteral("The saved story progress is missing. Reload the story.");
        return false;
    }

    for (const auto& change : result.delta.inventoryChanges) {
        const std::string itemId = std::to_string(change.itemId);
        const auto rowResult = connection_->query(
            "SELECT inventory_id, quantity FROM inventories "
            "WHERE character_id = ? AND item_id = ? FOR UPDATE",
            {characterId, itemId});
        if (!rowResult) {
            error = databaseError(connection_, QStringLiteral("An inventory item could not be locked."));
            return false;
        }
        const QueryRow* row = rowResult->rows.empty() ? nullptr : &rowResult->rows.front();
        if (!sameInventoryQuantity(row, currentState.inventory.value(change.itemId, 0))) {
            error = QStringLiteral("The inventory changed in another session. Reload the story.");
            return false;
        }
        const auto quantity = candidate.inventory.value(change.itemId, 0);
        bool saved = false;
        if (quantity == 0) {
            saved = !row || connection_->execute(
                                "DELETE FROM inventories WHERE inventory_id = ?",
                                {database::detail::value(*row, "inventory_id")});
        } else if (row) {
            saved = connection_->execute(
                "UPDATE inventories SET quantity = ? WHERE inventory_id = ?",
                {std::to_string(quantity), database::detail::value(*row, "inventory_id")});
        } else {
            saved = connection_->execute(
                "INSERT INTO inventories (character_id, item_id, quantity) VALUES (?, ?, ?)",
                {characterId, itemId, std::to_string(quantity)});
        }
        if (!saved) {
            error = databaseError(connection_, QStringLiteral("An inventory change could not be saved."));
            return false;
        }
    }

    QSet<QString> changedNpcs;
    for (const auto& change : result.delta.affinityChanges) changedNpcs.insert(change.npcId);
    for (const auto& change : result.delta.npcMemoryChanges) changedNpcs.insert(change.npcId);
    for (const auto& npcId : sorted(changedNpcs)) {
        const auto memory = connection_->query(
            "SELECT affinity, memory_flags, memory_json FROM npc_memories "
            "WHERE character_id = ? AND npc_id = ? FOR UPDATE",
            {characterId, npcId.toStdString()});
        if (!memory) {
            error = databaseError(connection_, QStringLiteral("An NPC memory could not be locked."));
            return false;
        }
        const QueryRow* row = memory->rows.empty() ? nullptr : &memory->rows.front();
        QSet<QString> storedFacts;
        if (row && !memoryFacts(*row, storedFacts, error)) return false;
        const int storedAffinity = row ? database::detail::integer<int>(*row, "affinity") : 0;
        if (storedAffinity != currentState.npcAffinities.value(npcId, 0)
            || storedFacts != currentState.npcMemoryFacts.value(npcId)) {
            error = QStringLiteral("NPC memory changed in another session. Reload the story.");
            return false;
        }
        const auto facts = candidate.npcMemoryFacts.value(npcId);
        const auto legacyFlags = sorted(facts).join(QLatin1Char(';')).toStdString();
        if (!connection_->execute(
                "INSERT INTO npc_memories "
                "(character_id, npc_id, affinity, memory_flags, memory_json, revision) "
                "VALUES (?, ?, ?, ?, ?, 1) ON DUPLICATE KEY UPDATE "
                "affinity = VALUES(affinity), memory_flags = VALUES(memory_flags), "
                "memory_json = VALUES(memory_json), revision = revision + 1",
                {characterId, npcId.toStdString(),
                 std::to_string(candidate.npcAffinities.value(npcId, 0)), legacyFlags,
                 updatedMemoryJson(row, facts, candidate.currentSceneId).toStdString()})) {
            error = databaseError(connection_, QStringLiteral("An NPC memory change could not be saved."));
            return false;
        }
    }

    if (result.delta.goldChange != 0
        && !connection_->execute(
            "UPDATE characters SET gold = ? WHERE character_id = ?",
            {std::to_string(candidate.gold), characterId})) {
        error = databaseError(connection_, QStringLiteral("The gold change could not be saved."));
        return false;
    }

    const std::string status = candidate.chapterCompleted ? "completed" : "active";
    if ((!candidate.chapterCompleted || !hasProgress)
        && !connection_->execute(
            "UPDATE character_story_states SET status = 'paused' "
            "WHERE character_id = ? AND chapter_id <> ? AND status = 'active'",
            {characterId, chapterId})) {
        error = databaseError(connection_, QStringLiteral("Other chapter progress could not be paused."));
        return false;
    }
    const auto flags = sorted(candidate.flags).join(QLatin1Char(';')).toStdString();
    const auto json = stateJson(candidate).toStdString();
    const std::string stage = candidate.chapterCompleted ? "completed" : "exploring";
    bool progressSaved = false;
    if (hasProgress) {
        progressSaved = connection_->execute(
            "UPDATE character_story_states SET content_version = ?, scene_id = ?, stage = ?, "
            "flags = ?, status = ?, state_json = ?, revision = ?, "
            "completed_at = CASE WHEN ? = 'completed' THEN COALESCE(completed_at, CURRENT_TIMESTAMP) "
            "ELSE NULL END WHERE character_id = ? AND chapter_id = ? AND revision = ?",
            {std::to_string(chapter.contentVersion), candidate.currentSceneId.toStdString(), stage,
             flags, status, json, std::to_string(candidate.revision), status,
             characterId, chapterId, std::to_string(currentState.revision)});
    } else {
        progressSaved = connection_->execute(
            "INSERT INTO character_story_states "
            "(character_id, chapter_id, content_version, scene_id, stage, flags, status, "
            "state_json, revision, completed_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, "
            "CASE WHEN ? = 'completed' THEN CURRENT_TIMESTAMP ELSE NULL END)",
            {characterId, chapterId, std::to_string(chapter.contentVersion),
             candidate.currentSceneId.toStdString(), stage, flags, status, json,
             std::to_string(candidate.revision), status});
    }
    if (!progressSaved) {
        error = databaseError(connection_, QStringLiteral("Story progress could not be saved."));
        return false;
    }

    const std::string actionId = result.actionId.isEmpty()
        ? "__enter_scene"
        : result.actionId.toStdString();
    if (!connection_->execute(
            "INSERT INTO story_event_log "
            "(character_id, chapter_id, scene_id, action_id, result_json) "
            "VALUES (?, ?, ?, ?, ?)",
            {characterId, chapterId, candidate.currentSceneId.toStdString(), actionId,
             eventJson(result, currentState, candidate.revision).toStdString()})) {
        error = databaseError(connection_, QStringLiteral("The story event could not be recorded."));
        return false;
    }

    if (!transaction.commit()) {
        error = databaseError(connection_, QStringLiteral("The story transaction could not be committed."));
        return false;
    }
    return true;
}

} // namespace arcane::application::story::persistence
