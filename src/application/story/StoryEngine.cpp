#include "application/story/StoryEngine.hpp"

#include <algorithm>
#include <limits>
#include <type_traits>

namespace arcane::application::story {

namespace {

void setReason(QString* destination, const QString& reason)
{
    if (destination) *destination = reason;
}

bool addSigned(std::uint64_t base, std::int64_t change, std::uint64_t& result)
{
    if (change < 0) {
        const auto magnitude = static_cast<std::uint64_t>(-(change + 1)) + 1;
        if (base < magnitude) return false;
        result = base - magnitude;
        return true;
    }
    const auto increase = static_cast<std::uint64_t>(change);
    if (base > std::numeric_limits<std::uint64_t>::max() - increase) return false;
    result = base + increase;
    return true;
}

bool addInt64(std::int64_t base, std::int64_t change, std::int64_t& result)
{
    if (change > 0 && base > std::numeric_limits<std::int64_t>::max() - change) return false;
    if (change < 0 && base < std::numeric_limits<std::int64_t>::min() - change) return false;
    result = base + change;
    return true;
}

bool hasVisitedScene(const StoryRuntimeState& state, const QString& sceneId)
{
    return state.visitedScenes.contains(sceneId)
        || state.flags.contains(QStringLiteral("visited_%1").arg(sceneId));
}

StoryEventKind eventKindFor(StoryLineKind kind)
{
    switch (kind) {
    case StoryLineKind::Narration: return StoryEventKind::Narration;
    case StoryLineKind::NpcDialogue: return StoryEventKind::NpcDialogue;
    case StoryLineKind::SystemNotice: return StoryEventKind::SystemNotice;
    }
    return StoryEventKind::Narration;
}

} // namespace

bool StoryEngine::conditionsSatisfied(const QVector<StoryCondition>& conditions,
                                      const StoryRuntimeState& state,
                                      QString* reason) const
{
    for (const auto& condition : conditions) {
        const bool satisfied = std::visit([&](const auto& value) -> bool {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, FlagCondition>) {
                return state.flags.contains(value.key) == value.expectedPresent;
            } else if constexpr (std::is_same_v<T, ItemQuantityCondition>) {
                return state.inventory.value(value.itemId, 0) >= value.minimumQuantity;
            } else if constexpr (std::is_same_v<T, GoldAtLeastCondition>) {
                return state.gold >= value.amount;
            } else if constexpr (std::is_same_v<T, AffinityAtLeastCondition>) {
                return state.npcAffinities.value(value.npcId, 0) >= value.minimum;
            } else if constexpr (std::is_same_v<T, AffinityAtMostCondition>) {
                return state.npcAffinities.value(value.npcId, 0) <= value.maximum;
            } else if constexpr (std::is_same_v<T, CharacterAttributeCondition>) {
                return state.characterAttributes.value(value.attribute) == value.expectedValue;
            } else if constexpr (std::is_same_v<T, ChapterCompletedCondition>) {
                return state.completedChapters.contains(value.chapterId);
            } else if constexpr (std::is_same_v<T, NpcMemoryCondition>) {
                return state.npcMemoryFacts.value(value.npcId).contains(value.key)
                    == value.expectedPresent;
            }
            return false;
        }, condition);
        if (!satisfied) {
            setReason(reason, QStringLiteral("A story condition is not satisfied."));
            return false;
        }
    }
    return true;
}

bool StoryEngine::canStart(const ChapterDefinition& chapter,
                           const StoryRuntimeState& state,
                           QString* reason) const
{
    return conditionsSatisfied(chapter.requirements, state, reason);
}

QVector<const StoryActionDefinition*> StoryEngine::availableActions(
    const ChapterDefinition& chapter, const StoryRuntimeState& state) const
{
    QVector<const StoryActionDefinition*> result;
    if (state.chapterCompleted && !chapter.allowActionsAfterCompletion) return result;
    const auto* scene = chapter.findScene(state.currentSceneId);
    if (!scene) return result;
    for (const auto& action : scene->actions) {
        if (conditionsSatisfied(action.conditions, state)) result.push_back(&action);
    }
    return result;
}

void StoryEngine::appendLineEvents(QVector<StoryEvent>& destination,
                                   const QVector<StoryLineDefinition>& lines,
                                   const StoryRuntimeState& state,
                                   const QString& sceneId) const
{
    for (const auto& line : lines) {
        if (!conditionsSatisfied(line.conditions, state)) continue;
        LocalizedStoryText speaker = line.speaker;
        if (speaker.isEmpty() && line.kind == StoryLineKind::Narration)
            speaker = {QStringLiteral("旁白"), QStringLiteral("Narrator")};
        if (speaker.isEmpty() && line.kind == StoryLineKind::SystemNotice)
            speaker = {QStringLiteral("系统"), QStringLiteral("System")};
        destination.push_back(
            {eventKindFor(line.kind), sceneId, line.actorId, speaker, line.text, {}});
    }
}

void StoryEngine::appendSceneEvents(QVector<StoryEvent>& destination,
                                    const SceneDefinition& scene,
                                    const StoryRuntimeState& state,
                                    bool firstVisit) const
{
    destination.push_back(
        {StoryEventKind::SceneHeader, scene.id, {}, {}, scene.title, {}});
    const auto& narration = !firstVisit && scene.revisitNarration.isComplete()
        ? scene.revisitNarration : scene.narration;
    destination.push_back(
        {StoryEventKind::Narration, scene.id, {},
         {QStringLiteral("旁白"), QStringLiteral("Narrator")}, narration, {}});
    appendLineEvents(destination, scene.arrivalLines, state, scene.id);
}

StoryEvent StoryEngine::choicesEvent(const ChapterDefinition& chapter,
                                     const StoryRuntimeState& state) const
{
    StoryEvent event;
    event.kind = StoryEventKind::ChoicesChanged;
    event.sceneId = state.currentSceneId;
    for (const auto* action : availableActions(chapter, state))
        event.choices.push_back({action->id, action->label});
    return event;
}

StoryExecutionResult StoryEngine::enterCurrentScene(
    const ChapterDefinition& chapter, const StoryRuntimeState& state) const
{
    if (!state.chapterId.isEmpty() && state.chapterId != chapter.chapterId)
        return {false, QStringLiteral("The runtime state belongs to another chapter."), {}, {}};
    const auto* scene = chapter.findScene(state.currentSceneId);
    if (!scene)
        return {false, QStringLiteral("The current story scene does not exist."), {}, {}};

    StoryExecutionResult result;
    result.success = true;
    result.message = QStringLiteral("Story scene prepared.");
    result.delta.scenesVisitedToAdd.insert(scene->id);

    StoryRuntimeState projectedState = state;
    QString error;
    if (!applyDelta(projectedState, result.delta, &error))
        return {false, error, {}, {}};
    appendSceneEvents(result.events, *scene, projectedState, !hasVisitedScene(state, scene->id));
    result.events.push_back(choicesEvent(chapter, projectedState));
    return result;
}

StoryExecutionResult StoryEngine::execute(const ChapterDefinition& chapter,
                                          const StoryRuntimeState& state,
                                          const QString& actionId) const
{
    if (state.chapterCompleted && !chapter.allowActionsAfterCompletion)
        return {false, QStringLiteral("The chapter is already complete."), actionId, {}};
    if (!state.chapterId.isEmpty() && state.chapterId != chapter.chapterId)
        return {false, QStringLiteral("The runtime state belongs to another chapter."), actionId, {}};

    const auto* scene = chapter.findScene(state.currentSceneId);
    if (!scene)
        return {false, QStringLiteral("The current story scene does not exist."), actionId, {}};

    const StoryActionDefinition* selected = nullptr;
    for (const auto& action : scene->actions) {
        if (action.id == actionId) {
            selected = &action;
            break;
        }
    }
    if (!selected)
        return {false, QStringLiteral("That action is not available in this scene."), actionId, {}};

    QString reason;
    if (!conditionsSatisfied(selected->conditions, state, &reason))
        return {false, reason, actionId, {}};

    StoryDelta delta;
    QHash<QString, std::int64_t> affinityTotals;
    QHash<std::uint64_t, std::int64_t> inventoryTotals;
    bool effectTotalsValid = true;
    for (const auto& effect : selected->effects) {
        std::visit([&](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, SetFlagEffect>) {
                if (value.present) {
                    delta.flagsToRemove.remove(value.key);
                    delta.flagsToAdd.insert(value.key);
                } else {
                    delta.flagsToAdd.remove(value.key);
                    delta.flagsToRemove.insert(value.key);
                }
            } else if constexpr (std::is_same_v<T, ChangeAffinityEffect>) {
                auto& total = affinityTotals[value.npcId];
                effectTotalsValid = effectTotalsValid && addInt64(total, value.amount, total);
            } else if constexpr (std::is_same_v<T, ChangeGoldEffect>) {
                effectTotalsValid = effectTotalsValid
                    && addInt64(delta.goldChange, value.amount, delta.goldChange);
            } else if constexpr (std::is_same_v<T, ChangeItemEffect>) {
                auto& total = inventoryTotals[value.itemId];
                effectTotalsValid = effectTotalsValid && addInt64(total, value.quantity, total);
            } else if constexpr (std::is_same_v<T, MoveToSceneEffect>) {
                delta.nextSceneId = value.sceneId;
            } else if constexpr (std::is_same_v<T, CompleteChapterEffect>) {
                delta.completeChapter = true;
            } else if constexpr (std::is_same_v<T, UnlockChapterEffect>) {
                if (!delta.chaptersToUnlock.contains(value.chapterId))
                    delta.chaptersToUnlock.push_back(value.chapterId);
            } else if constexpr (std::is_same_v<T, TransitionChapterEffect>) {
                delta.nextChapterId = value.chapterId;
                if (!delta.chaptersToUnlock.contains(value.chapterId))
                    delta.chaptersToUnlock.push_back(value.chapterId);
            } else if constexpr (std::is_same_v<T, SetNpcMemoryEffect>) {
                const auto duplicate = std::find_if(
                    delta.npcMemoryChanges.begin(), delta.npcMemoryChanges.end(),
                    [&](const NpcMemoryChange& change) {
                        return change.npcId == value.npcId && change.key == value.key;
                    });
                if (duplicate == delta.npcMemoryChanges.end())
                    delta.npcMemoryChanges.push_back({value.npcId, value.key, value.present});
                else
                    duplicate->present = value.present;
            }
        }, effect);
    }
    if (!effectTotalsValid)
        return {false, QStringLiteral("This action contains changes outside the supported range."), actionId, {}};
    if (delta.nextSceneId && !chapter.findScene(*delta.nextSceneId))
        return {false, QStringLiteral("This action points to a missing story scene."), actionId, {}};

    std::uint64_t projectedGold = 0;
    if (!addSigned(state.gold, delta.goldChange, projectedGold))
        return {false, QStringLiteral("This action would produce an invalid gold balance."), actionId, {}};

    for (auto it = inventoryTotals.cbegin(); it != inventoryTotals.cend(); ++it) {
        std::int64_t projected = 0;
        if (!addInt64(static_cast<std::int64_t>(state.inventory.value(it.key(), 0)),
                      it.value(), projected))
            return {false, QStringLiteral("This action contains an unsupported inventory change."), actionId, {}};
        if (projected < 0 || projected > std::numeric_limits<std::uint32_t>::max())
            return {false, QStringLiteral("This action would produce an invalid inventory quantity."), actionId, {}};
        if (it.value() < std::numeric_limits<std::int32_t>::min()
            || it.value() > std::numeric_limits<std::int32_t>::max())
            return {false, QStringLiteral("This action contains an unsupported inventory change."), actionId, {}};
        delta.inventoryChanges.push_back({it.key(), static_cast<std::int32_t>(it.value())});
    }
    for (auto it = affinityTotals.cbegin(); it != affinityTotals.cend(); ++it) {
        std::int64_t projected = 0;
        if (!addInt64(static_cast<std::int64_t>(state.npcAffinities.value(it.key(), 0)),
                      it.value(), projected))
            return {false, QStringLiteral("This action contains an unsupported affinity change."), actionId, {}};
        if (projected < std::numeric_limits<int>::min() || projected > std::numeric_limits<int>::max())
            return {false, QStringLiteral("This action would produce an invalid affinity value."), actionId, {}};
        if (it.value() < std::numeric_limits<int>::min()
            || it.value() > std::numeric_limits<int>::max())
            return {false, QStringLiteral("This action contains an unsupported affinity change."), actionId, {}};
        delta.affinityChanges.push_back({it.key(), static_cast<int>(it.value())});
    }

    if (!state.chapterCompleted && !chapter.completionConditions.isEmpty()) {
        StoryRuntimeState projectedState = state;
        QString applyError;
        if (!applyDelta(projectedState, delta, &applyError))
            return {false, applyError, actionId, {}};
        if (conditionsSatisfied(chapter.completionConditions, projectedState)) {
            delta.completeChapter = true;
            if (!chapter.completionFlag.isEmpty()) delta.flagsToAdd.insert(chapter.completionFlag);
        }
    }

    const QString destinationSceneId = delta.nextSceneId.value_or(state.currentSceneId);
    const bool entersNewScene = delta.nextSceneId.has_value();
    const bool firstDestinationVisit = entersNewScene
        && !hasVisitedScene(state, destinationSceneId);
    if (entersNewScene) delta.scenesVisitedToAdd.insert(destinationSceneId);

    StoryRuntimeState projectedState = state;
    QString projectionError;
    if (!applyDelta(projectedState, delta, &projectionError))
        return {false, projectionError, actionId, {}};

    QVector<StoryEvent> events;
    const LocalizedStoryText playerText = selected->playerText.isComplete()
        ? selected->playerText : selected->label;
    events.push_back(
        {StoryEventKind::PlayerAction, scene->id, {},
         {QStringLiteral("你"), QStringLiteral("You")}, playerText, {}});
    appendLineEvents(events, selected->responseLines, projectedState, scene->id);
    if (entersNewScene) {
        const auto* destination = chapter.findScene(destinationSceneId);
        appendSceneEvents(events, *destination, projectedState, firstDestinationVisit);
    }
    if (delta.completeChapter && !state.chapterCompleted) {
        events.push_back(
            {StoryEventKind::ChapterCompleted, projectedState.currentSceneId, {},
             {QStringLiteral("旁白"), QStringLiteral("Narrator")},
             chapter.completionText, {}});
    }
    events.push_back(choicesEvent(chapter, projectedState));

    return {true, QStringLiteral("Story action accepted."), actionId,
            std::move(delta), std::move(events)};
}

bool StoryEngine::applyDelta(StoryRuntimeState& state,
                             const StoryDelta& delta,
                             QString* error) const
{
    std::uint64_t updatedGold = 0;
    if (!addSigned(state.gold, delta.goldChange, updatedGold)) {
        setReason(error, QStringLiteral("Invalid gold balance."));
        return false;
    }

    auto updatedInventory = state.inventory;
    for (const auto& change : delta.inventoryChanges) {
        const std::int64_t projected = static_cast<std::int64_t>(updatedInventory.value(change.itemId, 0))
            + change.quantity;
        if (projected < 0 || projected > std::numeric_limits<std::uint32_t>::max()) {
            setReason(error, QStringLiteral("Invalid inventory quantity."));
            return false;
        }
        if (projected == 0) updatedInventory.remove(change.itemId);
        else updatedInventory[change.itemId] = static_cast<std::uint32_t>(projected);
    }

    auto updatedAffinities = state.npcAffinities;
    for (const auto& change : delta.affinityChanges) {
        const auto projected = static_cast<std::int64_t>(updatedAffinities.value(change.npcId, 0))
            + change.amount;
        if (projected < std::numeric_limits<int>::min() || projected > std::numeric_limits<int>::max()) {
            setReason(error, QStringLiteral("Invalid NPC affinity."));
            return false;
        }
        updatedAffinities[change.npcId] = static_cast<int>(projected);
    }

    auto updatedMemory = state.npcMemoryFacts;
    for (const auto& change : delta.npcMemoryChanges) {
        if (change.present) updatedMemory[change.npcId].insert(change.key);
        else {
            auto it = updatedMemory.find(change.npcId);
            if (it != updatedMemory.end()) {
                it->remove(change.key);
                if (it->isEmpty()) updatedMemory.erase(it);
            }
        }
    }

    state.gold = updatedGold;
    state.inventory = std::move(updatedInventory);
    state.npcAffinities = std::move(updatedAffinities);
    state.npcMemoryFacts = std::move(updatedMemory);
    for (const auto& flag : delta.flagsToRemove) state.flags.remove(flag);
    for (const auto& flag : delta.flagsToAdd) state.flags.insert(flag);
    for (const auto& chapterId : delta.chaptersToUnlock) state.unlockedChapters.insert(chapterId);
    if (delta.nextChapterId) state.unlockedChapters.insert(*delta.nextChapterId);
    for (const auto& sceneId : delta.scenesVisitedToAdd) state.visitedScenes.insert(sceneId);
    if (delta.nextSceneId) state.currentSceneId = *delta.nextSceneId;
    if (delta.completeChapter) {
        state.chapterCompleted = true;
        state.completedChapters.insert(state.chapterId);
    }
    return true;
}

} // namespace arcane::application::story
