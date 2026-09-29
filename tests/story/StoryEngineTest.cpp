#include "application/story/StoryContentLoader.hpp"
#include "application/story/StoryContentValidator.hpp"
#include "application/story/StoryEngine.hpp"

#include <QByteArray>

#include <iostream>

namespace story = arcane::application::story;

namespace {

bool expect(bool condition, const char* message)
{
    if (condition) return true;
    std::cerr << "FAILED: " << message << '\n';
    return false;
}

const QByteArray kValidChapter = R"json(
{
  "schema_version": 2,
  "chapter_id": "journey_contract_test",
  "content_version": 1,
  "entry_scene": "station",
  "title": {"zh": "测试旅程", "en": "Test Journey"},
  "requirements": [
    {"type": "chapter_completed", "chapter_id": "diagon_alley_shopping"}
  ],
  "scenes": [
    {
      "id": "station",
      "title": {"zh": "车站", "en": "Station"},
      "narration": {"zh": "蒸汽笼罩着站台。", "en": "Steam covers the platform."},
      "actors": ["elara_finch"],
      "actions": [
        {
          "id": "share_sweets",
          "label": {"zh": "分享零食", "en": "Share sweets"},
          "aliases": ["分享巧克力蛙", "share sweets"],
          "conditions": [
            {"type": "has_item", "item_id": 2, "minimum_quantity": 1},
            {"type": "gold_at_least", "amount": 5}
          ],
          "effects": [
            {"type": "change_item", "item_id": 2, "quantity": -1},
            {"type": "change_gold", "amount": -5},
            {"type": "change_affinity", "npc_id": "elara_finch", "amount": 2},
            {"type": "set_flag", "key": "shared_sweets"},
            {"type": "move_to_scene", "scene_id": "train"}
          ]
        }
      ]
    },
    {
      "id": "train",
      "title": {"zh": "列车", "en": "Train"},
      "narration": {"zh": "列车驶向北方。", "en": "The train travels north."},
      "actors": ["elara_finch"],
      "actions": [
        {
          "id": "finish_journey",
          "label": {"zh": "继续旅程", "en": "Continue the journey"},
          "aliases": ["继续", "continue"],
          "conditions": [{"type": "has_flag", "key": "shared_sweets"}],
          "effects": [
            {"type": "unlock_chapter", "chapter_id": "sorting_ceremony"},
            {"type": "complete_chapter"}
          ]
        }
      ]
    }
  ]
}
)json";

} // namespace

int main()
{
    bool passed = true;
    story::StoryContentLoader loader;
    const auto loaded = loader.loadJson(kValidChapter, QStringLiteral("valid-test"));
    passed &= expect(loaded.success(), "valid chapter JSON loads");
    if (!loaded.chapter) return 1;

    story::StoryContentValidator validator;
    const auto validation = validator.validate(*loaded.chapter);
    passed &= expect(validation.isValid(), "valid chapter passes semantic validation");
    passed &= expect(validation.warnings.isEmpty(), "complete test chapter has no warnings");

    story::StoryRuntimeState state;
    state.characterId = 42;
    state.chapterId = loaded.chapter->chapterId;
    state.currentSceneId = loaded.chapter->entrySceneId;
    state.gold = 10;
    state.inventory[2] = 1;

    story::StoryEngine engine;
    passed &= expect(!engine.canStart(*loaded.chapter, state),
                     "chapter prerequisite blocks an unqualified character");
    state.completedChapters.insert(QStringLiteral("diagon_alley_shopping"));
    passed &= expect(engine.canStart(*loaded.chapter, state),
                     "chapter prerequisite accepts completed prior chapter");

    const auto actions = engine.availableActions(*loaded.chapter, state);
    passed &= expect(actions.size() == 1 && actions.first()->id == QLatin1String("share_sweets"),
                     "condition evaluator exposes the valid scene action");

    const auto share = engine.execute(*loaded.chapter, state, QStringLiteral("share_sweets"));
    passed &= expect(share.success, "engine produces a delta for a valid action");
    passed &= expect(state.gold == 10 && state.inventory.value(2) == 1,
                     "delta generation has no side effects before persistence accepts it");
    passed &= expect(share.delta.goldChange == -5
                         && share.delta.inventoryChanges.size() == 1
                         && share.delta.affinityChanges.size() == 1
                         && share.delta.nextSceneId == QStringLiteral("train"),
                     "delta contains gold, inventory, affinity, flag, and movement changes");
    passed &= expect(engine.applyDelta(state, share.delta), "delta applies to in-memory state");
    passed &= expect(state.gold == 5 && !state.inventory.contains(2)
                         && state.npcAffinities.value(QStringLiteral("elara_finch")) == 2
                         && state.flags.contains(QStringLiteral("shared_sweets"))
                         && state.currentSceneId == QLatin1String("train"),
                     "applied state matches the declared effects");

    const auto finish = engine.execute(*loaded.chapter, state, QStringLiteral("finish_journey"));
    passed &= expect(finish.success && finish.delta.completeChapter,
                     "completion action produces chapter completion");
    passed &= expect(engine.applyDelta(state, finish.delta)
                         && state.chapterCompleted
                         && state.completedChapters.contains(QStringLiteral("journey_contract_test"))
                         && state.unlockedChapters.contains(QStringLiteral("sorting_ceremony")),
                     "completion and unlock effects update runtime state");

    const QByteArray invalidTarget = QByteArray(kValidChapter).replace(
        "\"scene_id\": \"train\"", "\"scene_id\": \"missing_scene\"");
    const auto badLoad = loader.loadJson(invalidTarget, QStringLiteral("invalid-target"));
    passed &= expect(badLoad.success(), "syntactically valid bad reference still loads");
    passed &= expect(badLoad.chapter && !validator.validate(*badLoad.chapter).isValid(),
                     "validator rejects missing and unreachable scene references");

    const QByteArray unknownEffect = QByteArray(kValidChapter).replace(
        "\"type\": \"change_gold\"", "\"type\": \"run_sql\"");
    const auto unsafeLoad = loader.loadJson(unknownEffect, QStringLiteral("unsafe-effect"));
    passed &= expect(!unsafeLoad.success(), "loader rejects effects outside the whitelist");

    story::StoryRuntimeState poorState;
    poorState.chapterId = loaded.chapter->chapterId;
    poorState.currentSceneId = loaded.chapter->entrySceneId;
    poorState.gold = 0;
    poorState.inventory[2] = 1;
    passed &= expect(!engine.execute(*loaded.chapter, poorState, QStringLiteral("share_sweets")).success,
                     "engine rejects an action whose runtime conditions fail");

    return passed ? 0 : 1;
}
