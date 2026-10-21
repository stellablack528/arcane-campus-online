# 开放世界状态快照任务报告

## 交付结果

本次完成了 HogwartsOnline 的第一块开放世界适配边界：将数据库恢复后的
`StoryRuntimeState` 转换为独立、只读、与 Provider 无关的 `OpenWorldSnapshot`。

当前数据路径为：

```text
数据库事务恢复
  → StoryRuntimeState（现有权威状态）
  → OpenWorldSnapshotMapper
  → OpenWorldSnapshot（未来 TaleState 的只读输入）
```

本轮没有接入 TaleState 库、模型 Provider 或网络请求，也没有让开放世界逻辑接管玩家输入。

## 代码结构

新增目录保持在独立的 `application/open_world` 边界中：

```text
src/application/open_world/
├── model/
│   └── OpenWorldSnapshot.hpp
└── mapping/
    ├── OpenWorldSnapshotMapper.hpp
    └── OpenWorldSnapshotMapper.cpp

tests/open_world/
└── OpenWorldSnapshotMapperTest.cpp
```

构建系统新增 `ArcaneOpenWorldBoundary` 静态库。它和现有剧情引擎分开，方便后续把 TaleState
适配实现限定在开放世界目录，不把外部引擎类型扩散到控制器、UI 或数据库层。

## 快照包含的世界事实

- 角色 ID、当前章节、内容版本和场景；
- 当前章节已经接受的剧情回合数；
- 学院及完整角色属性；
- 金币和背包；
- 全部剧情标记；
- NPC 好感关系和记忆事实；
- 已访问场景；
- 已完成章节和已解锁章节；
- 当前章节是否完成；
- `main_story_complete` 与 `open_world_handoff_ready` 交接状态。

快照只使用 C++ 标准库类型，不依赖 Qt、数据库、网络或 TaleState。Qt 容器和字符串的转换被限制在
`OpenWorldSnapshotMapper.cpp` 内。

现有剧情状态还没有单独的通用 `completed_events` 集合，因此本轮没有把 flags 或章节错误地伪装成
TaleState 事件。后续建立 TaleState 状态映射时，应按霍格沃茨世界包的事件定义进行显式转换。

## 安全与一致性

- 映射器只读取源状态，不执行 SQL，也不会修改 `StoryRuntimeState`。
- 角色 ID、章节、场景或内容版本无效时拒绝生成快照。
- 仅有 `open_world_handoff_ready`、却缺少 `main_story_complete` 时视为损坏状态并拒绝交接。
- 普通主线状态仍然可以生成快照，但 `readyForOpenWorld()` 返回 `false`，不会提前开放探索。
- 当前 `revision` 每次在接受并提交一个剧情行动后递增，边界中用
  `acceptedStoryTurns` 表达这层业务含义，不向未来适配器暴露数据库术语。

## 玩家体验

本轮属于底层地基，玩家当前看到的内容没有变化：

- 对角巷、霍格沃茨旅程和分院仪式继续按原流程运行；
- 新生序章结束提示和自主探索输入状态保持不变；
- 尚未启用生成式移动、交谈或 NPC 自由对话。

完成这一步后，未来开放世界引擎已经可以获得一份完整、稳定且不直接连接数据库的玩家世界快照。

## 测试结果

- 主程序和全部测试目标编译通过。
- CTest：9/9 通过。
- 新增快照专项：11/11 检查通过。
- 旧数据库、剧情引擎、事务恢复、对角巷等价性、完整主线和分院仪式测试全部继续通过。
- 固定测试结果：`tests/results/open_world_snapshot_report.txt`。

本轮未运行需要真实 MySQL 实例的可选集成测试；默认数据库冒烟和内存事务测试均已通过。

## 未改动范围

- 未修改 TCP、Socket 或联机协议。
- 未修改数据库 Schema 或迁移。
- 未引入 API Key 或模型配置。
- 未引入 TaleState 构建依赖。
- 未调用 TaleState `StoryAgent`、`ILlmProvider` 或 `check_connection()`。
- 未修改中央叙事框和玩家主流程。

## 下一步接口

下一次可以在当前快照之上实现单向状态映射：

```text
OpenWorldSnapshot
  → TaleState StoryState
```

届时仍然只做内存转换和契约测试，不需要立即接模型或网络。
