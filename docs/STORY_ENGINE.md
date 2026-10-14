# Generic Story Engine / 通用剧情引擎

The generic story engine is the contract between authored or agent-generated content and the C++ game runtime. It now owns the production Diagon Alley route: chapter content is adapted into the generic model, restored from the database, committed as atomic `StoryDelta` transactions, and only then published to the central narrative UI. The legacy runtime remains compiled only by regression tests.

通用剧情引擎是“人工或 Agent 生成的内容”和“C++ 游戏运行时”之间的边界。对角巷玩家入口现已正式切换：旧内容通过适配器映射为通用章节，数据库恢复和 `StoryDelta` 事务成为唯一生产状态路径；旧运行时只保留在回归测试中，用于持续验证新旧完整游玩结果等价。

## Responsibility boundary

```text
chapter JSON
  -> StoryContentLoader       syntax and whitelist parsing
  -> StoryContentValidator    ids, references, graph reachability, bilingual text
  -> StoryEngine              conditions -> StoryDelta + ordered StoryEvent stream
  -> StoryRuntimeService      commit first -> publish signals second
  -> persistence layer        one atomic database transaction (production integration)
  -> central narrative UI     render committed result only
```

The engine never executes SQL, calls an AI model, or writes UI widgets. A content pack can only select from the condition and effect types compiled into the engine.

## Supported conditions

| JSON type | Meaning |
|---|---|
| `has_flag` / `missing_flag` | Check a chapter fact |
| `has_item` | Require an inventory quantity |
| `gold_at_least` | Require available Galleons |
| `affinity_at_least` / `affinity_at_most` | Require an NPC relationship range |
| `character_attribute_is` | Check a whitelisted character attribute |
| `chapter_completed` | Require an earlier chapter |
| `npc_remembers` / `npc_missing_memory` | Check one stable NPC memory fact |

Whitelisted character attributes are `house`, `blood_status`, `wand_wood`, `wand_core`, and `pet_type`.

## Supported effects

| JSON type | Result in `StoryDelta` |
|---|---|
| `set_flag` / `remove_flag` | Add or remove a chapter fact |
| `change_affinity` | Adjust one NPC relationship |
| `change_gold` | Add or subtract Galleons |
| `change_item` | Grant or consume an item |
| `move_to_scene` | Select the next scene |
| `complete_chapter` | Complete the current chapter |
| `unlock_chapter` | Unlock another chapter |
| `set_npc_memory` / `remove_npc_memory` | Add or remove one NPC memory fact |

Unknown types are rejected while loading. Content can never provide arbitrary C++, SQL, shell commands, or network requests.

## Narrative output

State changes and presentation are separate outputs of one accepted action:

- `StoryDelta` describes what must be persisted atomically.
- Ordered `StoryEvent` values describe what the player should see only after persistence succeeds.

The event stream supports `SceneHeader`, `Narration`, `NpcDialogue`, `PlayerAction`, `SystemNotice`, `ChoicesChanged`, and `ChapterCompleted`. Content may declare `player_text`, conditional `response_lines`, scene `arrival_lines`, and `revisit_narration`. NPC lines carry both a stable actor ID and bilingual display name. `enterCurrentScene` produces the same scene/choice contract for a new or resumed session and records visited scenes through a delta.

This keeps Qt widgets chapter-agnostic: they render committed events and never contain rules for a particular shop, NPC, or chapter.

## Runtime service and UI bridge

`StoryRuntimeService` owns one active chapter session and is the safety boundary between the deterministic engine, persistence, and Qt. It accepts button choices or free-text aliases, asks `StoryEngine` for a `StoryDelta` and ordered events, applies the delta to a temporary state, and invokes an injected `DeltaCommitter`. Only a successful commit replaces the live state and releases the event stream.

If validation, execution, in-memory application, or persistence fails, the service emits `runtimeError` but does not emit scene, narration, choice, or state-change signals. This prevents the central narrative frame from showing an action that the database rejected.

The service maps generic events onto chapter-independent Qt signals:

- scene headers -> `sceneHeaderChanged`
- narration, NPC dialogue, player action, system notice, and completion -> `narrativeProduced`
- available actions -> `choicesPresented`
- committed state -> `runtimeStateChanged`

`CampusController` bridges those signals into the existing central narrative frame and routes both typed intent and choice buttons to the service. Completing the wand prologue starts the adapted chapter immediately; login restores the generic save, while a one-time compatibility seed preserves the scene and flags from legacy `diagon_alley` rows. Gold and inventory widgets refresh from the committed runtime state.

## Validation

`StoryContentValidator` rejects content with invalid or duplicate IDs, missing bilingual text, broken scene references, multiple transitions in one action, unreachable scenes, or no completion rule. A chapter may complete through an explicit `complete_chapter` effect or declarative `completion_conditions`; the latter can also add a `completion_flag`. Chapters such as Diagon Alley may opt into post-completion exploration with `allow_actions_after_completion`. The machine-readable structural contract is `src/resources/story/schemas/chapter_v2.schema.json`; semantic graph checks remain in C++.

## Persistence v4

`migrate_v4_generic_story_engine.sql` changes story progress to a `(character_id, chapter_id)` key, adds content version, status, JSON state, revision and timestamps, adds structured NPC memory fields, and creates the append-only `story_event_log` table. Existing semicolon flags are retained under `legacy_flags` for the Phase 2 compatibility adapter.

`DBTransaction` is an RAII guard: leaving scope without a successful commit rolls the transaction back.

`StoryPersistenceService` is the database adapter used by `StoryRuntimeService`. For every accepted action it:

1. locks and verifies the character and chapter revision;
2. applies gold, inventory, NPC affinity/memory, and chapter progress changes;
3. appends a structured `story_event_log` record;
4. commits all writes together, or rolls all of them back when any step fails.

The revision check rejects a stale client/session instead of silently overwriting newer progress. On restore, the adapter rebuilds `StoryRuntimeState` from the character, inventory, wand, NPC memory, and all relevant chapter rows. It validates the content version and saved scene before the runtime can publish anything.

Apply all migrations with:

```powershell
cd database/scripts
.\import_mysql.ps1
```

## Diagon Alley production cutover and parity gate

`DiagonAlleyChapterAdapter` reads the already-loaded v1 catalog and produces a validated v2 `ChapterDefinition` in memory. It does not duplicate story text or maintain a second content file. This adapted chapter is now the production entry.

`ArcaneCampusDiagonAlleyParityTest` completes the same route through the legacy service and generic engine, then compares gold, inventory, flags, NPC affinity, NPC memory, final scene, and chapter completion. It writes a human-readable text report with `--report <path>`.

The legacy `DiagonAlleyService` is no longer linked into the application target. It remains in the parity and legacy runtime test targets only, so the same route can keep checking gold, inventory, flags, NPC affinity, memory, final scene, chapter completion, typed input, button routing, and resume behavior.
