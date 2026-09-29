# Generic Story Engine / 通用剧情引擎

The generic story engine is the contract between authored or agent-generated content and the C++ game runtime. Phase 1 provides the contract, JSON loader, semantic validator, deterministic condition/effect execution, `StoryDelta`, and database transaction foundation. The existing Diagon Alley runtime remains active until its parity migration in Phase 2.

通用剧情引擎是“人工或 Agent 生成的内容”和“C++ 游戏运行时”之间的边界。第一阶段只搭建基础设施，不改变玩家当前看到的对角巷流程。

## Responsibility boundary

```text
chapter JSON
  -> StoryContentLoader       syntax and whitelist parsing
  -> StoryContentValidator    ids, references, graph reachability, bilingual text
  -> StoryEngine              conditions -> deterministic StoryDelta
  -> persistence layer        one atomic database transaction (Phase 2 integration)
  -> UI                       render committed result only
```

The engine never executes SQL, calls an AI model, or writes UI widgets. A content pack can only select from the condition and effect types compiled into the engine.

## Supported conditions

| JSON type | Meaning |
|---|---|
| `has_flag` / `missing_flag` | Check a chapter fact |
| `has_item` | Require an inventory quantity |
| `gold_at_least` | Require available Galleons |
| `affinity_at_least` | Require an NPC relationship value |
| `character_attribute_is` | Check a whitelisted character attribute |
| `chapter_completed` | Require an earlier chapter |

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

Unknown types are rejected while loading. Content can never provide arbitrary C++, SQL, shell commands, or network requests.

## Validation

`StoryContentValidator` rejects content with invalid or duplicate IDs, missing bilingual text, broken scene references, multiple transitions in one action, unreachable scenes, or no completion effect. The machine-readable structural contract is `src/resources/story/schemas/chapter_v2.schema.json`; semantic graph checks remain in C++.

## Persistence v4

`migrate_v4_generic_story_engine.sql` changes story progress to a `(character_id, chapter_id)` key, adds content version, status, JSON state, revision and timestamps, adds structured NPC memory fields, and creates the append-only `story_event_log` table. Existing semicolon flags are retained under `legacy_flags` for the Phase 2 compatibility adapter.

`DBTransaction` is an RAII guard: leaving scope without a successful commit rolls the transaction back. Phase 2 will use it to commit gold, inventory, NPC memory, chapter progress, and event logging together.

Apply all migrations with:

```powershell
cd database/scripts
.\import_mysql.ps1
```

## Phase boundary

Phase 1 deliberately does not route `CampusController` or `DiagonAlleyService` through the generic engine. Phase 2 first converts the current Diagon Alley package, compares complete playthrough results, and removes its dedicated runtime only after parity tests pass.
