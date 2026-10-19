# 剧情内容包 / Story Content Packs

对角巷章节的剧情文本、商店、NPC 与商品不再散落在界面或控制器代码中，而是集中在
`src/resources/data/diagon_alley.json`。运行时由 `DiagonAlleyCatalog` 读取，动作解析器和剧情服务只负责解释与执行。

> 当前对角巷文件仍是第一版专用格式。`DiagonAlleyChapterAdapter` 会在内存中把它映射为
> 通用章节 v2，并由完整游玩等价性测试守门，因此剧情文本仍只有这一份。
> 详细契约见 `src/resources/story/schemas/chapter_v2.schema.json` 与 `docs/STORY_ENGINE.md`。

这层边界是未来“异世界生成 Agent”的入口：Agent 只需产出经过校验的内容包，不需要生成或改写 C++ 业务代码。

## 顶层结构

| 字段 | 作用 |
|---|---|
| `schema_version` | 内容包格式版本；当前为 `1` |
| `chapter_id` | 稳定的章节标识，不随显示语言改变 |
| `chapter_title` | 章节中英文名称 |
| `street` | 主场景标题、首次/再次进入文案、完成文案 |
| `shops` | 可探索商店列表 |

所有显示文本使用 `{ "zh": "…", "en": "…" }`。内部 ID 使用小写英文与下划线，并在发布后保持稳定，这样存档升级时不会丢失进度。

## 商店结构

每个商店包含：

- `id`：场景与存档使用的稳定 ID。
- `title`、`intro`、`revisit`：名称、首次进入与重访叙事。
- `keywords`：中英文自由输入的识别词。
- `npc`：店主身份，以及普通、友好、警惕三类招呼和交谈/忽略反馈。
- `purchases`：商品 ID、识别词、数据库物品 ID、价格、完成标记和成功文案。

`completion_flag` 是采购清单的状态键。当前章节必须最终具备 `wand`、`books`、`robes`、`potions`、`pet`，才会写入 `shopping_complete`。

## 运行时职责

```text
diagon_alley.json
  → DiagonAlleyCatalog（读取与查询）
  └→ DiagonAlleyChapterAdapter ┐
journey_to_hogwarts.json ───────┴→ StoryCampaignService → StoryRuntimeService
  → StoryEngine → StoryPersistenceService → CampusController（中央叙事框与状态面板）
```

AI 可用于生成风格化叙事，但不能直接决定扣款、发放物品或修改存档。所有会改变角色状态的动作仍由确定性的本地规则执行。

通用章节 v2 还可以声明 `revisit_narration`、`arrival_lines`、`player_text` 和
`response_lines`。引擎会把它们转换成有序的 `StoryEvent`，并根据好感、记忆、物品或
剧情标记筛选条件台词；界面只负责渲染已经提交成功的事件。

跨章节使用 `transition_to_chapter` 效果。它会先把目标章节写入解锁状态并完成当前动作的
数据库事务，成功后才请求章节编排服务进入目标章节。目标必须已在 `StoryChapterCatalog`
注册，否则整个内置战役在加载阶段就会被拒绝。仅预告未来内容而不立即进入时，使用
`unlock_chapter`。

## 新增内容检查清单

1. ID 不与已有章节、商店、NPC、商品重复。
2. 中英文显示文本均存在。
3. `keywords` 至少包含一个中文词和一个英文词。
4. `item_id` 已出现在数据库种子或迁移脚本中。
5. 价格与新生初始金币能够形成可完成的闭环。
6. 新增的完成标记已纳入章节完成条件。
7. 运行 `ArcaneCampusDiagonAlleyRuntimeTest`，确认旧入口内容包、自由输入、NPC 记忆和采购闭环均通过。
8. 运行 `ArcaneCampusDiagonAlleyParityTest`，确认旧入口和通用引擎的完整结果仍等价。
9. 运行 `ArcaneCampusStoryCampaignTest`，确认跨章节、NPC 记忆、宠物分支、恢复和最终解锁均通过。
