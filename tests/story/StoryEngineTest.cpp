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
  "completion_text": {"zh": "旅程完成。", "en": "Journey complete."},
  "scenes": [
    {
      "id": "station",
      "title": {"zh": "车站", "en": "Station"},
      "narration": {"zh": "蒸汽笼罩着站台。", "en": "Steam covers the platform."},
      "revisit_narration": {"zh": "蒸汽再次漫过脚边。", "en": "Steam curls around your feet again."},
      "actors": ["elara_finch"],
      "arrival_lines": [
        {
          "type": "npc_dialogue",
          "actor_id": "elara_finch",
          "speaker": {"zh": "埃拉拉", "en": "Elara"},
          "text": {"zh": "这里还有座位。", "en": "There is a seat here."},
          "conditions": [
            {"type": "affinity_at_least", "npc_id": "elara_finch", "minimum": 0},
            {"type": "affinity_at_most", "npc_id": "elara_finch", "maximum": 0}
          ]
        }
      ],
      "actions": [
        {
          "id": "share_sweets",
          "label": {"zh": "分享零食", "en": "Share sweets"},
          "player_text": {"zh": "你递出一只巧克力蛙。", "en": "You offer a Chocolate Frog."},
          "aliases": ["分享巧克力蛙", "share sweets"],
          "conditions": [
            {"type": "has_item", "item_id": 2, "minimum_quantity": 1},
            {"type": "gold_at_least", "amount": 5},
            {"type": "npc_missing_memory", "npc_id": "elara_finch", "key": "shared_snack"}
          ],
          "effects": [
            {"type": "change_item", "item_id": 2, "quantity": -1},
            {"type": "change_gold", "amount": -5},
            {"type": "change_affinity", "npc_id": "elara_finch", "amount": 2},
            {"type": "set_npc_memory", "npc_id": "elara_finch", "key": "shared_snack"},
            {"type": "set_flag", "key": "shared_sweets"},
            {"type": "move_to_scene", "scene_id": "train"}
          ],
          "response_lines": [
            {
              "type": "npc_dialogue",
              "actor_id": "elara_finch",
              "speaker": {"zh": "埃拉拉", "en": "Elara"},
              "text": {"zh": "谢谢你。", "en": "Thank you."},
              "conditions": [
                {"type": "affinity_at_least", "npc_id": "elara_finch", "minimum": 2}
              ]
            },
            {
              "type": "system_notice",
              "text": {"zh": "零食和加隆已经扣除。", "en": "The sweets and Galleons were spent."}
            }
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
          "conditions": [
            {"type": "has_flag", "key": "shared_sweets"},
            {"type": "npc_remembers", "npc_id": "elara_finch", "key": "shared_snack"}
          ],
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

    const auto opening = engine.enterCurrentScene(*loaded.chapter, state);
    passed &= expect(opening.success && opening.events.size() == 4
                         && opening.events.at(0).kind == story::StoryEventKind::SceneHeader
                         && opening.events.at(1).kind == story::StoryEventKind::Narration
                         && opening.events.at(2).kind == story::StoryEventKind::NpcDialogue
                         && opening.events.at(3).kind == story::StoryEventKind::ChoicesChanged,
                     "scene entry produces header, narration, conditional NPC dialogue, and choices");
    passed &= expect(engine.applyDelta(state, opening.delta)
                         && state.visitedScenes.contains(QStringLiteral("station")),
                     "scene presentation records the visit through a delta");
    const auto revisit = engine.enterCurrentScene(*loaded.chapter, state);
    passed &= expect(revisit.success && revisit.events.at(1).text.zh.contains(QStringLiteral("再次")),
                     "a repeated scene entry uses revisit narration");

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
                         && share.delta.npcMemoryChanges.size() == 1
                         && share.delta.scenesVisitedToAdd.contains(QStringLiteral("train"))
                         && share.delta.nextSceneId == QStringLiteral("train"),
                     "delta contains gold, inventory, affinity, flag, and movement changes");
    passed &= expect(share.events.size() == 6
                         && share.events.at(0).kind == story::StoryEventKind::PlayerAction
                         && share.events.at(1).kind == story::StoryEventKind::NpcDialogue
                         && share.events.at(2).kind == story::StoryEventKind::SystemNotice
                         && share.events.at(3).kind == story::StoryEventKind::SceneHeader
                         && share.events.at(4).kind == story::StoryEventKind::Narration
                         && share.events.at(5).kind == story::StoryEventKind::ChoicesChanged,
                     "action output preserves player, response, transition, and choice event order");
    passed &= expect(engine.applyDelta(state, share.delta), "delta applies to in-memory state");
    passed &= expect(state.gold == 5 && !state.inventory.contains(2)
                         && state.npcAffinities.value(QStringLiteral("elara_finch")) == 2
                         && state.npcMemoryFacts.value(QStringLiteral("elara_finch"))
                                .contains(QStringLiteral("shared_snack"))
                         && state.flags.contains(QStringLiteral("shared_sweets"))
                         && state.currentSceneId == QLatin1String("train"),
                     "applied state matches the declared effects");

    const auto finish = engine.execute(*loaded.chapter, state, QStringLiteral("finish_journey"));
    passed &= expect(finish.success && finish.delta.completeChapter,
                     "completion action produces chapter completion");
    passed &= expect(finish.events.size() == 3
                         && finish.events.at(1).kind == story::StoryEventKind::ChapterCompleted
                         && finish.events.at(1).text.en == QLatin1String("Journey complete.")
                         && finish.events.at(2).choices.isEmpty(),
                     "completion output includes chapter text and clears unavailable choices");
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

    const QByteArray unknownLine = QByteArray(kValidChapter).replace(
        "\"type\": \"system_notice\"", "\"type\": \"run_script\"");
    const auto unsafeLineLoad = loader.loadJson(unknownLine, QStringLiteral("unsafe-line"));
    passed &= expect(!unsafeLineLoad.success(),
                     "loader rejects narrative output types outside the whitelist");

    story::StoryRuntimeState poorState;
    poorState.chapterId = loaded.chapter->chapterId;
    poorState.currentSceneId = loaded.chapter->entrySceneId;
    poorState.gold = 0;
    poorState.inventory[2] = 1;
    passed &= expect(!engine.execute(*loaded.chapter, poorState, QStringLiteral("share_sweets")).success,
                     "engine rejects an action whose runtime conditions fail");

    return passed ? 0 : 1;
}
