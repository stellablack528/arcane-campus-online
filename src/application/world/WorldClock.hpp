#pragma once

#include <string>

namespace arcane::application::world {

// 校园世界时间系统：把一天划分为明确的时段，每个时段有不同的玩法影响。
//
// 设计原则：
// - 纯 C++ 类，不依赖 Qt（便于未来迁移到 TCP Server / Linux 后端评估事件）。
// - 时段只做"判断"，不直接产生消息或副作用（消息反馈由 WorldEvent 负责）。
enum class TimePeriod {
    EarlyMorning,      // 06:00-08:00 早晨（早餐、准备）
    MorningClasses,    // 08:00-12:00 上午课程
    Lunch,             // 12:00-14:00 午餐
    AfternoonClasses,  // 14:00-18:00 下午课程
    Evening,           // 18:00-21:00 傍晚（自由活动）
    Night,             // 21:00-24:00 夜晚（宵禁前）
    LateNight,         // 00:00-06:00 深夜（宵禁中）
};

class WorldClock final {
public:
    // 从 24 小时制的 hour（0-23）计算 TimePeriod。
    [[nodiscard]] static TimePeriod periodFromHour(int hour);

    // 人类可读的时段名称（如 "Morning Classes"）。
    [[nodiscard]] static std::string displayName(TimePeriod period);

    // 时段属性查询接口（为后续 WorldEvent / 地点逻辑预留）。
    [[nodiscard]] static bool classesInSession(TimePeriod period);   // 当前是否上课时间
    [[nodiscard]] static bool publicAreasOpen(TimePeriod period);    // 公共区域是否开放
    [[nodiscard]] static bool commonRoomOnly(TimePeriod period);    // 宵禁仅限宿舍
    [[nodiscard]] static bool quietHours(TimePeriod period);        // 夜间安静时段
};

} // namespace arcane::application::world
