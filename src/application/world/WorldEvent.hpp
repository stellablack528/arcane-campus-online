#pragma once

#include "application/world/WorldClock.hpp"

#include <string>
#include <vector>

namespace arcane::application::world {

// 世界事件的评估上下文。评估事件时所需的最小信息集。
//
// 设计为 POD：未来 TCP Server 可以用同样的结构构造 context 评估事件，
// 不需要依赖 Qt 或客户端 session 对象。
struct WorldEventContext {
    TimePeriod period = TimePeriod::EarlyMorning;
    std::string locationId;        // 玩家当前地点 id（如 "library"、"great_hall"），空表示位置未知
    bool playerOnline = false;     // 玩家是否已登录
};

// 事件执行后产生的反馈。message 会作为世界消息显示在聊天区。
struct WorldEventResult {
    std::string speaker;   // 说话者（如 "System"、"Library"、"Hermione Granger"）
    std::string message;   // 消息内容
};

// 世界事件抽象接口。
//
// 设计原则（约束 AI 边界）：
// - 事件负责"判断条件 + 产生消息反馈"，不直接修改数据库游戏状态。
// - AI 只负责"角色扮演和对话"，不通过 AI 决定游戏核心状态。
// - 事件应当纯 C++（无 Qt 依赖），便于在 TCP Server / Linux 后端复用。
class WorldEvent {
public:
    virtual ~WorldEvent() = default;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual bool matches(const WorldEventContext& context) const = 0;
    [[nodiscard]] virtual std::vector<WorldEventResult> execute(const WorldEventContext& context) const = 0;
};

// ─── 可运行的示例事件 ───────────────────────────────────────────

// 示例 1：深夜宵禁（LateNight 0:00-6:00 且玩家在线）
//   公共区域关闭，提示玩家宵禁生效，prefects 巡逻。
class CurfewEvent final : public WorldEvent {
public:
    [[nodiscard]] std::string name() const override { return "Curfew"; }
    [[nodiscard]] bool matches(const WorldEventContext& context) const override;
    [[nodiscard]] std::vector<WorldEventResult> execute(const WorldEventContext& context) const override;
};

// 示例 2：夜晚图书馆夜间自习（Night 21:00-24:00 且玩家在 library）
//   图书馆进入夜间特殊氛围，有 NPC 提示限制区的存在。
class NightStudyEvent final : public WorldEvent {
public:
    [[nodiscard]] std::string name() const override { return "Night Study"; }
    [[nodiscard]] bool matches(const WorldEventContext& context) const override;
    [[nodiscard]] std::vector<WorldEventResult> execute(const WorldEventContext& context) const override;
};

// 评估所有已注册事件，返回所有满足条件的执行结果。
// 当前是硬编码示例事件，未来可扩展为从配置加载事件列表。
[[nodiscard]] std::vector<WorldEventResult> evaluateWorldEvents(const WorldEventContext& context);

} // namespace arcane::application::world
