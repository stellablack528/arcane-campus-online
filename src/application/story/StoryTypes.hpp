#pragma once

#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include <cstdint>
#include <optional>
#include <variant>

namespace arcane::application::story {

enum class StoryLanguage {
    Chinese,
    English,
};

struct LocalizedStoryText {
    QString zh;
    QString en;

    [[nodiscard]] bool isComplete() const
    {
        return !zh.trimmed().isEmpty() && !en.trimmed().isEmpty();
    }

    [[nodiscard]] bool isEmpty() const
    {
        return zh.trimmed().isEmpty() && en.trimmed().isEmpty();
    }

    [[nodiscard]] QString resolve(StoryLanguage language) const
    {
        return language == StoryLanguage::Chinese ? zh : en;
    }
};

struct FlagCondition {
    QString key;
    bool expectedPresent = true;
};

struct ItemQuantityCondition {
    std::uint64_t itemId = 0;
    std::uint32_t minimumQuantity = 1;
};

struct GoldAtLeastCondition {
    std::uint64_t amount = 0;
};

struct AffinityAtLeastCondition {
    QString npcId;
    int minimum = 0;
};

struct AffinityAtMostCondition {
    QString npcId;
    int maximum = 0;
};

struct CharacterAttributeCondition {
    QString attribute;
    QString expectedValue;
};

struct ChapterCompletedCondition {
    QString chapterId;
};

struct NpcMemoryCondition {
    QString npcId;
    QString key;
    bool expectedPresent = true;
};

using StoryCondition = std::variant<
    FlagCondition,
    ItemQuantityCondition,
    GoldAtLeastCondition,
    AffinityAtLeastCondition,
    AffinityAtMostCondition,
    CharacterAttributeCondition,
    ChapterCompletedCondition,
    NpcMemoryCondition>;

struct SetFlagEffect {
    QString key;
    bool present = true;
};

struct ChangeAffinityEffect {
    QString npcId;
    int amount = 0;
};

struct ChangeGoldEffect {
    std::int64_t amount = 0;
};

struct ChangeItemEffect {
    std::uint64_t itemId = 0;
    std::int32_t quantity = 0;
};

struct MoveToSceneEffect {
    QString sceneId;
};

struct CompleteChapterEffect {
};

struct UnlockChapterEffect {
    QString chapterId;
};

struct TransitionChapterEffect {
    QString chapterId;
};

struct SetNpcMemoryEffect {
    QString npcId;
    QString key;
    bool present = true;
};

using StoryEffect = std::variant<
    SetFlagEffect,
    ChangeAffinityEffect,
    ChangeGoldEffect,
    ChangeItemEffect,
    MoveToSceneEffect,
    CompleteChapterEffect,
    UnlockChapterEffect,
    TransitionChapterEffect,
    SetNpcMemoryEffect>;

enum class StoryLineKind {
    Narration,
    NpcDialogue,
    SystemNotice,
};

struct StoryLineDefinition {
    StoryLineKind kind = StoryLineKind::Narration;
    QString actorId;
    LocalizedStoryText speaker;
    LocalizedStoryText text;
    QVector<StoryCondition> conditions;
};

struct StoryActionDefinition {
    QString id;
    LocalizedStoryText label;
    LocalizedStoryText playerText;
    QStringList aliases;
    QVector<StoryCondition> conditions;
    QVector<StoryEffect> effects;
    QVector<StoryLineDefinition> responseLines;
};

struct SceneDefinition {
    QString id;
    LocalizedStoryText title;
    LocalizedStoryText narration;
    LocalizedStoryText revisitNarration;
    QStringList actorIds;
    QVector<StoryLineDefinition> arrivalLines;
    QVector<StoryActionDefinition> actions;
};

struct ChapterDefinition {
    int schemaVersion = 0;
    QString chapterId;
    int contentVersion = 0;
    QString entrySceneId;
    LocalizedStoryText title;
    QVector<StoryCondition> requirements;
    QVector<StoryCondition> completionConditions;
    QString completionFlag;
    LocalizedStoryText completionText;
    bool allowActionsAfterCompletion = false;
    QVector<SceneDefinition> scenes;

    [[nodiscard]] const SceneDefinition* findScene(const QString& sceneId) const
    {
        for (const auto& scene : scenes) {
            if (scene.id == sceneId) return &scene;
        }
        return nullptr;
    }
};

struct StoryRuntimeState {
    std::uint64_t characterId = 0;
    QString chapterId;
    int contentVersion = 0;
    std::uint64_t revision = 0;
    QString currentSceneId;
    std::uint64_t gold = 0;
    QSet<QString> flags;
    QHash<std::uint64_t, std::uint32_t> inventory;
    QHash<QString, int> npcAffinities;
    QHash<QString, QSet<QString>> npcMemoryFacts;
    QHash<QString, QString> characterAttributes;
    QSet<QString> visitedScenes;
    QSet<QString> completedChapters;
    QSet<QString> unlockedChapters;
    bool chapterCompleted = false;
};

struct AffinityChange {
    QString npcId;
    int amount = 0;
};

struct InventoryChange {
    std::uint64_t itemId = 0;
    std::int32_t quantity = 0;
};

struct NpcMemoryChange {
    QString npcId;
    QString key;
    bool present = true;
};

// A deterministic description of one accepted action. It contains no SQL and
// makes no external calls; persistence applies it atomically in a later layer.
struct StoryDelta {
    std::optional<QString> nextSceneId;
    std::optional<QString> nextChapterId;
    QSet<QString> flagsToAdd;
    QSet<QString> flagsToRemove;
    QVector<AffinityChange> affinityChanges;
    QVector<InventoryChange> inventoryChanges;
    QVector<NpcMemoryChange> npcMemoryChanges;
    std::int64_t goldChange = 0;
    QStringList chaptersToUnlock;
    QSet<QString> scenesVisitedToAdd;
    bool completeChapter = false;

    [[nodiscard]] bool isEmpty() const
    {
        return !nextSceneId && !nextChapterId
            && flagsToAdd.isEmpty() && flagsToRemove.isEmpty()
            && affinityChanges.isEmpty() && inventoryChanges.isEmpty()
            && npcMemoryChanges.isEmpty() && goldChange == 0
            && chaptersToUnlock.isEmpty() && scenesVisitedToAdd.isEmpty()
            && !completeChapter;
    }
};

enum class StoryEventKind {
    SceneHeader,
    Narration,
    NpcDialogue,
    PlayerAction,
    SystemNotice,
    ChoicesChanged,
    ChapterCompleted,
};

struct StoryChoicePresentation {
    QString actionId;
    LocalizedStoryText label;
};

// UI-facing, language-neutral output. The runtime resolves localized text and
// forwards these events; widgets never need chapter-specific C++ branches.
struct StoryEvent {
    StoryEventKind kind = StoryEventKind::Narration;
    QString sceneId;
    QString actorId;
    LocalizedStoryText speaker;
    LocalizedStoryText text;
    QVector<StoryChoicePresentation> choices;
};

struct StoryExecutionResult {
    bool success = false;
    QString message;
    QString actionId;
    StoryDelta delta;
    QVector<StoryEvent> events;
};

} // namespace arcane::application::story
