#include "application/world/WorldEvent.hpp"

namespace arcane::application::world {

bool CurfewEvent::matches(const WorldEventContext& context) const
{
    // 深夜时段且玩家在线时触发。
    return context.period == TimePeriod::LateNight && context.playerOnline;
}

std::vector<WorldEventResult> CurfewEvent::execute(const WorldEventContext& context) const
{
    (void)context;
    return {
        {"System", "The clock chimes midnight. Curfew is now in effect — public areas of the castle are closed until dawn."},
        {"System", "Prefects patrol the corridors. Return to your common room if you wish to avoid trouble."}
    };
}

bool NightStudyEvent::matches(const WorldEventContext& context) const
{
    // 夜晚时段且玩家当前在图书馆。
    return context.period == TimePeriod::Night
        && context.locationId == "library"
        && context.playerOnline;
}

std::vector<WorldEventResult> NightStudyEvent::execute(const WorldEventContext& context) const
{
    (void)context;
    return {
        {"Library", "Candles dim to a soft amber. The library settles into its hushed night-study hours — perfect for serious research."},
        {"Hermione Granger", "Finally, some quiet. If you need the Restricted Section indexes, ask me before Madam Pince notices."}
    };
}

std::vector<WorldEventResult> evaluateWorldEvents(const WorldEventContext& context)
{
    // 已注册的示例事件。后续可以替换为 vector<unique_ptr<WorldEvent>> 从配置加载。
    static const CurfewEvent curfew;
    static const NightStudyEvent nightStudy;

    std::vector<WorldEventResult> results;
    if (curfew.matches(context)) {
        const auto produced = curfew.execute(context);
        results.insert(results.end(), produced.begin(), produced.end());
    }
    if (nightStudy.matches(context)) {
        const auto produced = nightStudy.execute(context);
        results.insert(results.end(), produced.begin(), produced.end());
    }
    return results;
}

} // namespace arcane::application::world
