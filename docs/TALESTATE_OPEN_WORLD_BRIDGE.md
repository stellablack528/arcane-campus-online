# 开放世界与 TaleState 的第一段接线

参考仓库：[TaleState C++](https://github.com/stellablack528/talestate-cpp)，
固定到提交 `f5f11c08bd2269fdec8c80642c4d9aeea1eaa83e`。
为了让本项目离线也能构建，只引入 TaleState 的领域模型和确定性运行时；
对应文件及 MIT 许可位于 `third_party/talestate_core/`。

目前玩家在自由探索时点击地点，流程是：

1. `CampusController` 把当前地点和目标地点交给 `OpenWorldRouteGuard`。
2. 适配器构造 TaleState 的 `move` 请求。TaleState 根据声明的场景与路线判定是否合法。
3. 允许后，原有 `CampusService` 查询房间并移动角色；随后更新中央叙事框、状态栏和联机位置。

当前声明了魔咒课教室作为入学后的出发点，以及大礼堂、变形术教室、图书馆、庭院、霍格莫德五个可移动场景。未声明的地点会被拒绝。TaleState 在这一步只负责校验，不直接写数据库，也不接管聊天或已完成的主线章节。

下一步需要把路线和 NPC 规则移到可校验的世界包，并将现有存档快照映射为 TaleState 状态；在此基础上再接入 Agent 的自然语言规划、规则裁决与叙事生成。AI Key 不参与当前的移动校验。
