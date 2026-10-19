#include "application/story/StoryContentValidator.hpp"

#include <QQueue>
#include <QRegularExpression>
#include <QSet>

#include <type_traits>

namespace arcane::application::story {

namespace {

const QRegularExpression& stableIdPattern()
{
    static const QRegularExpression pattern(QStringLiteral("^[a-z][a-z0-9_]*$"));
    return pattern;
}

bool isStableId(const QString& value)
{
    return stableIdPattern().match(value).hasMatch();
}

void validateCondition(const StoryCondition& condition,
                       const QString& context,
                       StoryValidationResult& result)
{
    std::visit([&](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, FlagCondition>) {
            if (!isStableId(value.key)) result.errors << context + QStringLiteral(": invalid flag key.");
        } else if constexpr (std::is_same_v<T, ItemQuantityCondition>) {
            if (value.itemId == 0 || value.minimumQuantity == 0)
                result.errors << context + QStringLiteral(": item condition requires positive values.");
        } else if constexpr (std::is_same_v<T, AffinityAtLeastCondition>) {
            if (!isStableId(value.npcId)) result.errors << context + QStringLiteral(": invalid NPC id.");
        } else if constexpr (std::is_same_v<T, AffinityAtMostCondition>) {
            if (!isStableId(value.npcId)) result.errors << context + QStringLiteral(": invalid NPC id.");
        } else if constexpr (std::is_same_v<T, CharacterAttributeCondition>) {
            static const QSet<QString> allowed{
                QStringLiteral("house"), QStringLiteral("blood_status"),
                QStringLiteral("wand_wood"), QStringLiteral("wand_core"),
                QStringLiteral("pet_type")};
            if (!allowed.contains(value.attribute) || value.expectedValue.trimmed().isEmpty())
                result.errors << context + QStringLiteral(": unsupported or empty character attribute.");
        } else if constexpr (std::is_same_v<T, ChapterCompletedCondition>) {
            if (!isStableId(value.chapterId))
                result.errors << context + QStringLiteral(": invalid required chapter id.");
        } else if constexpr (std::is_same_v<T, NpcMemoryCondition>) {
            if (!isStableId(value.npcId) || !isStableId(value.key))
                result.errors << context + QStringLiteral(": invalid NPC memory reference.");
        }
    }, condition);
}

void validateEffect(const StoryEffect& effect,
                    const QString& context,
                    const QSet<QString>& sceneIds,
                    StoryValidationResult& result)
{
    std::visit([&](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, SetFlagEffect>) {
            if (!isStableId(value.key)) result.errors << context + QStringLiteral(": invalid flag key.");
        } else if constexpr (std::is_same_v<T, ChangeAffinityEffect>) {
            if (!isStableId(value.npcId) || value.amount == 0)
                result.errors << context + QStringLiteral(": affinity effect requires an NPC and non-zero amount.");
        } else if constexpr (std::is_same_v<T, ChangeGoldEffect>) {
            if (value.amount == 0) result.errors << context + QStringLiteral(": gold change cannot be zero.");
        } else if constexpr (std::is_same_v<T, ChangeItemEffect>) {
            if (value.itemId == 0 || value.quantity == 0)
                result.errors << context + QStringLiteral(": item effect requires an item and non-zero quantity.");
        } else if constexpr (std::is_same_v<T, MoveToSceneEffect>) {
            if (!sceneIds.contains(value.sceneId))
                result.errors << context + QStringLiteral(": target scene '%1' does not exist.").arg(value.sceneId);
        } else if constexpr (std::is_same_v<T, UnlockChapterEffect>) {
            if (!isStableId(value.chapterId))
                result.errors << context + QStringLiteral(": invalid unlocked chapter id.");
        } else if constexpr (std::is_same_v<T, TransitionChapterEffect>) {
            if (!isStableId(value.chapterId))
                result.errors << context + QStringLiteral(": invalid chapter transition id.");
        } else if constexpr (std::is_same_v<T, SetNpcMemoryEffect>) {
            if (!isStableId(value.npcId) || !isStableId(value.key))
                result.errors << context + QStringLiteral(": invalid NPC memory effect.");
        }
    }, effect);
}

void validateLine(const StoryLineDefinition& line,
                  const QString& context,
                  const QSet<QString>& sceneActorIds,
                  StoryValidationResult& result)
{
    if (!line.text.isComplete())
        result.errors << context + QStringLiteral(": text must be bilingual.");
    if (!line.speaker.isEmpty() && !line.speaker.isComplete())
        result.errors << context + QStringLiteral(": speaker must be bilingual when provided.");
    if (line.kind == StoryLineKind::NpcDialogue) {
        if (!isStableId(line.actorId))
            result.errors << context + QStringLiteral(": NPC dialogue requires a valid actor id.");
        else if (!sceneActorIds.contains(line.actorId))
            result.errors << context + QStringLiteral(": NPC actor is not present in the scene.");
        if (!line.speaker.isComplete())
            result.errors << context + QStringLiteral(": NPC dialogue requires a bilingual speaker.");
    } else if (!line.actorId.isEmpty()) {
        result.errors << context + QStringLiteral(": only NPC dialogue may declare an actor id.");
    }
    for (qsizetype index = 0; index < line.conditions.size(); ++index) {
        validateCondition(line.conditions.at(index),
                          context + QStringLiteral(" condition %1").arg(index), result);
    }
}

} // namespace

StoryValidationResult StoryContentValidator::validate(const ChapterDefinition& chapter) const
{
    StoryValidationResult result;
    if (chapter.schemaVersion != 2)
        result.errors << QStringLiteral("schema_version must be 2.");
    if (!isStableId(chapter.chapterId))
        result.errors << QStringLiteral("chapter_id must be a stable lowercase id.");
    if (chapter.contentVersion <= 0)
        result.errors << QStringLiteral("content_version must be positive.");
    if (!chapter.title.isComplete())
        result.errors << QStringLiteral("chapter title requires Chinese and English text.");
    if (chapter.scenes.isEmpty())
        result.errors << QStringLiteral("chapter must contain at least one scene.");

    for (qsizetype index = 0; index < chapter.requirements.size(); ++index) {
        validateCondition(chapter.requirements.at(index),
                          QStringLiteral("chapter requirement %1").arg(index), result);
    }
    for (qsizetype index = 0; index < chapter.completionConditions.size(); ++index) {
        validateCondition(chapter.completionConditions.at(index),
                          QStringLiteral("chapter completion condition %1").arg(index), result);
    }
    if (!chapter.completionFlag.isEmpty() && !isStableId(chapter.completionFlag))
        result.errors << QStringLiteral("completion_flag must be a stable lowercase id.");
    if (chapter.completionConditions.isEmpty() && !chapter.completionFlag.isEmpty())
        result.errors << QStringLiteral("completion_flag requires completion_conditions.");
    if (!chapter.completionText.isEmpty() && !chapter.completionText.isComplete())
        result.errors << QStringLiteral("completion_text must be bilingual when provided.");
    if (chapter.completionText.isEmpty())
        result.warnings << QStringLiteral("chapter has no completion_text presentation.");

    QSet<QString> sceneIds;
    for (const auto& scene : chapter.scenes) {
        if (!isStableId(scene.id)) {
            result.errors << QStringLiteral("scene has invalid id '%1'.").arg(scene.id);
        } else if (sceneIds.contains(scene.id)) {
            result.errors << QStringLiteral("duplicate scene id '%1'.").arg(scene.id);
        }
        sceneIds.insert(scene.id);
    }
    if (!sceneIds.contains(chapter.entrySceneId))
        result.errors << QStringLiteral("entry scene '%1' does not exist.").arg(chapter.entrySceneId);

    QSet<QString> actionIds;
    bool hasCompletion = false;
    QHash<QString, QSet<QString>> edges;
    for (const auto& scene : chapter.scenes) {
        if (!scene.title.isComplete() || !scene.narration.isComplete())
            result.errors << QStringLiteral("scene '%1' requires bilingual title and narration.").arg(scene.id);
        if (!scene.revisitNarration.isEmpty() && !scene.revisitNarration.isComplete())
            result.errors << QStringLiteral("scene '%1' revisit narration must be bilingual.").arg(scene.id);
        QSet<QString> sceneActorIds;
        for (const auto& actorId : scene.actorIds) {
            if (!isStableId(actorId))
                result.errors << QStringLiteral("scene '%1' contains invalid actor id '%2'.")
                                     .arg(scene.id, actorId);
            sceneActorIds.insert(actorId);
        }
        for (qsizetype index = 0; index < scene.arrivalLines.size(); ++index)
            validateLine(scene.arrivalLines.at(index),
                         QStringLiteral("scene '%1' arrival line %2").arg(scene.id).arg(index),
                         sceneActorIds, result);
        if (scene.actions.isEmpty())
            result.warnings << QStringLiteral("scene '%1' has no actions.").arg(scene.id);

        for (const auto& action : scene.actions) {
            const QString context = QStringLiteral("scene '%1', action '%2'").arg(scene.id, action.id);
            if (!isStableId(action.id)) result.errors << context + QStringLiteral(": invalid action id.");
            if (actionIds.contains(action.id)) result.errors << context + QStringLiteral(": duplicate action id.");
            actionIds.insert(action.id);
            if (!action.label.isComplete()) result.errors << context + QStringLiteral(": label must be bilingual.");
            if (!action.playerText.isEmpty() && !action.playerText.isComplete())
                result.errors << context + QStringLiteral(": player_text must be bilingual when provided.");
            if (action.aliases.isEmpty()) result.warnings << context + QStringLiteral(": no free-text aliases.");
            if (action.effects.isEmpty()) result.warnings << context + QStringLiteral(": action has no effects.");

            for (qsizetype index = 0; index < action.conditions.size(); ++index) {
                validateCondition(action.conditions.at(index),
                                  context + QStringLiteral(" condition %1").arg(index), result);
            }

            int movementEffects = 0;
            int chapterTransitionEffects = 0;
            for (qsizetype index = 0; index < action.effects.size(); ++index) {
                const auto& effect = action.effects.at(index);
                validateEffect(effect, context + QStringLiteral(" effect %1").arg(index), sceneIds, result);
                if (const auto* movement = std::get_if<MoveToSceneEffect>(&effect)) {
                    ++movementEffects;
                    edges[scene.id].insert(movement->sceneId);
                }
                if (std::holds_alternative<TransitionChapterEffect>(effect))
                    ++chapterTransitionEffects;
                if (std::holds_alternative<CompleteChapterEffect>(effect)) hasCompletion = true;
            }
            if (movementEffects > 1)
                result.errors << context + QStringLiteral(": action has more than one scene transition.");
            if (chapterTransitionEffects > 1)
                result.errors << context + QStringLiteral(": action has more than one chapter transition.");
            if (movementEffects > 0 && chapterTransitionEffects > 0)
                result.errors << context + QStringLiteral(": action cannot change scene and chapter together.");
            for (qsizetype index = 0; index < action.responseLines.size(); ++index)
                validateLine(action.responseLines.at(index),
                             context + QStringLiteral(" response line %1").arg(index),
                             sceneActorIds, result);
        }
    }
    if (!hasCompletion && chapter.completionConditions.isEmpty())
        result.errors << QStringLiteral("chapter has no completion rule.");

    if (sceneIds.contains(chapter.entrySceneId)) {
        QSet<QString> reachable{chapter.entrySceneId};
        QQueue<QString> queue;
        queue.enqueue(chapter.entrySceneId);
        while (!queue.isEmpty()) {
            const QString current = queue.dequeue();
            for (const auto& target : edges.value(current)) {
                if (reachable.contains(target)) continue;
                reachable.insert(target);
                queue.enqueue(target);
            }
        }
        for (const auto& sceneId : sceneIds) {
            if (!reachable.contains(sceneId))
                result.errors << QStringLiteral("scene '%1' is unreachable from the entry scene.").arg(sceneId);
        }
    }

    return result;
}

} // namespace arcane::application::story
