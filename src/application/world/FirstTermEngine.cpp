#include "application/world/FirstTermEngine.hpp"

#include <storyforge/runtime/story_runtime.hpp>

#include <algorithm>
#include <cstdint>
#include <utility>
#include <variant>

namespace arcane::application::world {
namespace {

std::string advance(FirstTermState& state)
{
    if (++state.phase < 7) return {};
    state.phase = 0;
    if (++state.day > 35) {
        state.complete = true;
        return {};
    }
    if (state.day <= 7) {
        switch (state.day) {
        case 2: return "\n【周二清晨】楼梯口贴出了新的课表。有人边走边背咒语，礼堂里仍留着早餐的香气。";
        case 3: return "\n【周三清晨】魔咒课的练习越来越细。教授提醒大家，稳定比响亮的咒语更重要。";
        case 4: return "\n【周四清晨】图书馆的旧借阅册被人重新放回桌上，一张写着铜鸟编号的纸条露出一角。";
        case 5: return "\n【周五清晨】魔咒教室的窗边摆着一只练习用铜鸟。它的翅膀偶尔闪出不自然的蓝光。";
        case 6: return state.accidentOutcome == "missed"
            ? "\n【周六清晨】昨天的课堂意外传遍了礼堂。有人问起你为何没有到场。"
            : "\n【周六清晨】昨天的课堂意外传遍了礼堂。同学们谈起了你当时的举动。";
        case 7: return state.clueFound
            ? "\n【周日清晨】那条借阅线索仍夹在你的笔记里。下一次见到当事人，你有了可以追问的事。"
            : "\n【周日清晨】铜鸟失控的原因仍无人说清。图书馆的旧借阅册也许还留着答案。";
        default: break;
        }
    }
    if ((state.day - 1) % 7 != 0) return {};
    switch ((state.day - 1) / 7 + 1) {
    case 2: return "\n【第二周】课程渐渐加快，同学们在图书馆组织了小小的复习会。";
    case 3: return "\n【第三周】温室里的植物长势异常，草药课后仍有人留下观察。";
    case 4: return "\n【第四周】禁林边缘有了新的脚印。教授提醒你，任何冒险都要考虑身体状况。";
    case 5: return "\n【第五周】学期阶段考核临近，你的出勤、声望和各科积累开始显出差别。";
    default: return {};
    }
}

storyforge::StoryPackage forestPackage()
{
    using namespace storyforge;
    StoryPackage package;
    package.format = "story-package/v1";
    package.id = StoryId{"first-term-forest"};
    package.version = "1";
    package.entry = {ChapterId{"first_term"}, SceneId{"forbidden_forest"}, NodeId{"edge"}};
    StoryChapter chapter;
    chapter.id = package.entry.chapter;
    chapter.entry_scene = package.entry.scene;
    chapter.scenes.push_back({SceneId{"forbidden_forest"}, "禁林边缘", NodeId{"edge"},
                              {{NodeId{"edge"}, "树影中的脚步声忽近忽远。", {}, false}}});
    package.chapters.push_back(std::move(chapter));
    package.variables.push_back({VariableId{"health"}, std::int64_t{90}});
    package.variables.push_back({VariableId{"reputation"}, std::int64_t{0}});
    package.agent_actions.push_back({ChoiceId{"careful"}, "谨慎观察", {},
        {StoryEffect{ModifyVariableEffect{VariableId{"health"}, VariableOperation::add, std::int64_t{-12}}},
         StoryEffect{ModifyVariableEffect{VariableId{"reputation"}, VariableOperation::add, std::int64_t{2}}}}, {}});
    package.agent_actions.push_back({ChoiceId{"deep"}, "深入禁林", {},
        {StoryEffect{ModifyVariableEffect{VariableId{"health"}, VariableOperation::add, std::int64_t{-38}}},
         StoryEffect{ModifyVariableEffect{VariableId{"reputation"}, VariableOperation::add, std::int64_t{4}}}}, {}});
    return package;
}

TermOutcome reject(const FirstTermState& state, const char* reason)
{
    return {false, state, reason};
}

} // namespace

int FirstTermEngine::week(const FirstTermState& state) noexcept { return std::min(5, (state.day - 1) / 7 + 1); }
int FirstTermEngine::weekday(const FirstTermState& state) noexcept { return (state.day - 1) % 7; }

int FirstTermEngine::subjectFor(const FirstTermState& state) noexcept
{
    if (weekday(state) >= 5 || (state.phase != 1 && state.phase != 3)) return -1;
    return ((state.day - 1) * 2 + (state.phase == 3 ? 1 : 0)) % 4;
}

const char* FirstTermEngine::subjectName(int subject) noexcept
{
    static constexpr std::array names{"魔咒学", "变形术", "草药学", "黑魔法防御术"};
    return subject >= 0 && subject < 4 ? names[subject] : "";
}

const char* FirstTermEngine::classroomFor(int subject) noexcept
{
    static constexpr std::array rooms{"charms_classroom", "transfiguration_classroom",
                                      "greenhouse_three", "defense_classroom"};
    return subject >= 0 && subject < 4 ? rooms[subject] : "";
}

const char* FirstTermEngine::phaseName(const FirstTermState& state) noexcept
{
    static constexpr std::array names{"早餐 07:30", "上午 09:00", "午餐 12:30",
                                      "下午 14:00", "晚餐 18:00", "自由活动 19:00", "夜间 22:00"};
    return state.complete ? "五周结束" : names[std::clamp(state.phase, 0, 6)];
}

TermOutcome FirstTermEngine::apply(const FirstTermState& state, TermAction action,
                                    const std::string& locationId,
                                    const storyforge::StoryPackage* eraPackage)
{
    if (state.complete) return reject(state, "第一学期前五周已经结束。");
    FirstTermState next = state;
    if (state.accidentPending) {
        const char* actionId = action == TermAction::AccidentCast ? "action.accident.cast"
            : action == TermAction::AccidentHelp ? "action.accident.help"
            : action == TermAction::AccidentIgnore ? "action.accident.ignore" : nullptr;
        if (!actionId || !eraPackage) return reject(state, "请先处理眼前的课堂意外。");
        auto worldState = storyforge::make_initial_state(*eraPackage);
        worldState.variables[storyforge::VariableId{"var.charms"}] = std::int64_t{state.subjects[0]};
        worldState.variables[storyforge::VariableId{"var.absent"}] = false;
        worldState.variables[storyforge::VariableId{"var.reputation"}] = std::int64_t{state.reputation};
        const auto applied = storyforge::execute_agent_requests(*eraPackage, worldState,
            {{storyforge::AgentActionKind::authored, actionId, {}}});
        if (std::holds_alternative<storyforge::RuntimeError>(applied))
            return reject(state, "你的魔咒水平还不足以稳稳托住铜鸟；可以向教授求助，或先退开。");
        next.reputation = std::clamp(static_cast<int>(std::get<std::int64_t>(
            worldState.variables.at(storyforge::VariableId{"var.reputation"}))), -100, 100);
        next.accidentPending = false;
        next.accidentOutcome = action == TermAction::AccidentCast ? "cast"
            : action == TermAction::AccidentHelp ? "help" : "ignore";
        const std::string npc = eraPackage->agent_npcs.empty() ? "一位同学"
            : eraPackage->agent_npcs.front().name;
        return {true, next, next.accidentOutcome == "cast"
            ? "你念出漂浮咒，铜鸟在撞上玻璃前停住。" + npc + "长出一口气，记住了你的出手。"
            : next.accidentOutcome == "help"
                ? "你提醒教授，并拉开身旁同学。铜鸟被控制住，" + npc + "向你点头致谢。"
                : "你退到安全位置。教授处理了失控的铜鸟，" + npc + "看了你一眼，没有说话。"};
    }
    if (state.critical) {
        if (action != TermAction::Recover) return reject(state, "你已重伤，先接受校医救治。");
        next.critical = false;
        next.health = 35;
        next.reputation = std::max(-100, next.reputation - 4);
        next.forestChoice = false;
        next.phase = 6;
        return {true, next, "校医将你从危险边缘救回。你失去了一些声望，今晚只能休息。"};
    }
    if (state.forestChoice) {
        if (action == TermAction::ForestRetreat) {
            next.forestChoice = false;
            const auto beat = advance(next);
            return {true, next, "你听见枝叶间不明生物的动静，选择原路退回庭院。" + beat};
        }
        if (action != TermAction::ForestCareful && action != TermAction::ForestDeep)
            return reject(state, "先决定如何处理禁林里的危险。");
        static const auto package = forestPackage();
        auto forestState = storyforge::make_initial_state(package);
        forestState.variables[storyforge::VariableId{"health"}] = std::int64_t{state.health};
        forestState.variables[storyforge::VariableId{"reputation"}] = std::int64_t{state.reputation};
        const auto result = storyforge::execute_agent_requests(package, forestState,
            {{storyforge::AgentActionKind::authored,
              action == TermAction::ForestDeep ? "deep" : "careful", {}}});
        if (const auto* error = std::get_if<storyforge::RuntimeError>(&result))
            return reject(state, error->message.c_str());
        next.health = std::max(0, static_cast<int>(std::get<std::int64_t>(forestState.variables.at(storyforge::VariableId{"health"}))));
        next.reputation = std::clamp(static_cast<int>(std::get<std::int64_t>(forestState.variables.at(storyforge::VariableId{"reputation"}))), -100, 100);
        next.forestChoice = false;
        if (next.health == 0) {
            next.critical = true;
            return {true, next, "禁林深处的袭击让你失去意识，幸好巡夜教授及时发现了你。生命垂危，必须接受校医救治。"};
        }
        const auto beat = advance(next);
        return {true, next, action == TermAction::ForestDeep
            ? "你冒险深入禁林，带回一段罕见的见闻，也带着明显的伤。" + beat
            : "你保持距离观察，记下一点线索，但还是被荆棘划伤。" + beat};
    }

    if (action == TermAction::EnterForest) {
        if (state.phase != 5 || locationId != "courtyard")
            return reject(state, "只有自由活动时从庭院才能接近禁林。");
        next.forestChoice = true;
        return {true, next, "禁林边缘传来低沉的声响。越往里走，受伤甚至丧命的风险越高；你可以谨慎观察、深入，或立即撤离。"};
    }
    if (action == TermAction::Skip) {
        if (state.phase == 0 || state.phase == 2 || state.phase == 4)
            next.health = std::max(0, next.health - 12);
        else if (subjectFor(state) >= 0) {
            ++next.missed;
            next.reputation = std::max(-100, next.reputation - 3);
            if (eraPackage && state.day == 5 && state.phase == 1
                && subjectFor(state) == 0) next.accidentOutcome = "missed";
        } else if (state.phase == 6) next.health = std::max(0, next.health - 5);
        if (next.health == 0) {
            next.critical = true;
            return {true, next, "长期缺餐或熬夜使你晕倒。校医赶来救治，必须先接受治疗。"};
        }
        const auto beat = advance(next);
        return {true, next, std::string("这段时间过去了。缺席课程、错过饭点或熬夜都会留下影响。")
            + (next.accidentOutcome == "missed" && state.accidentOutcome.empty()
                ? "你错过的魔咒课上，练习用的铜鸟突然失控了。" : "") + beat};
    }
    if (state.phase == 0 || state.phase == 2 || state.phase == 4) {
        if (action != TermAction::Eat || locationId != "great_hall")
            return reject(state, "现在是饭点，请先到大礼堂用餐。");
        next.health = std::min(100, next.health + 8);
        ++next.meals;
        const auto beat = advance(next);
        return {true, next, "你按时在礼堂吃了一餐，精神恢复了一些。" + beat};
    }
    const int subject = subjectFor(state);
    if (subject >= 0) {
        if (action != TermAction::Attend || locationId != classroomFor(subject))
            return reject(state, "这节课已经开始，请前往对应教室上课。");
        next.subjects[subject] = std::min(100, next.subjects[subject] + 5);
        next.reputation = std::min(100, next.reputation + 1);
        ++next.attended;
        const auto beat = advance(next);
        if (eraPackage && state.day == 5 && state.phase == 1 && subject == 0) {
            next.accidentPending = true;
            return {true, next, std::string("你按时完成了") + subjectName(subject)
                + "课程。忽然，练习用的铜鸟冲向窗边，翅膀迸出蓝色火花！"};
        }
        return {true, next, std::string("你按时完成了") + subjectName(subject) + "课程，老师记住了你的认真。" + beat};
    }
    if (state.phase == 6) {
        if (action != TermAction::Rest) return reject(state, "夜深了，回宿舍休息或选择跳过这一晚。");
        next.health = std::min(100, next.health + 5);
        const auto beat = advance(next);
        return {true, next, "你睡了一夜，身体和思绪都轻松了些。" + beat};
    }
    if (action != TermAction::Explore) return reject(state, "现在可以自由探索，也可以跳过这段时间。");
    std::string exploration;
    if (locationId == "great_hall")
        exploration = "你在礼堂听同学分享传闻，又讲出了自己的见闻；有人开始记住你的名字。";
    else if (locationId == "library")
        exploration = week(state) == 2
            ? "你参加第二周的图书馆复习会，帮同学找回一本遗失的课本。"
            : "你在图书馆帮同学查到一条线索，几个人认真听完了你的解释。";
    else if (locationId == "courtyard")
        exploration = week(state) == 4
            ? "你在庭院查看禁林外的新脚印，也提醒同学别独自越过边界。"
            : "你在庭院和同学练习咒语，周围的人注意到了你的进步。";
    else if (locationId == "hogsmeade")
        exploration = "你在霍格莫德的街道上听人谈起校园近况，带回了有用的消息。";
    else
        exploration = "你观察了这处场景，暂时没有发现会改变别人看法的新线索。";
    if (eraPackage && state.day <= 7 && locationId == "library" && !state.clueFound) {
        auto worldState = storyforge::make_initial_state(*eraPackage);
        worldState.position.scene = storyforge::SceneId{"scene.library"};
        if (!std::holds_alternative<storyforge::RuntimeError>(storyforge::execute_agent_requests(
                *eraPackage, worldState, {{storyforge::AgentActionKind::authored,
                    "action.clue.find", {}}}))) {
            next.clueFound = true;
            const std::string clue = eraPackage->agent_objects.empty()
                ? "旧借阅记录" : eraPackage->agent_objects.front().name;
            exploration = "你在旧书堆里找到“" + clue + "”，上面留着与失控铜鸟有关的借阅线索。";
        }
    }
    if (eraPackage && state.day <= 7 && locationId == "great_hall"
        && !state.accidentOutcome.empty()
        && (!state.followupSeen || (state.clueFound && !state.clueShared))) {
        auto worldState = storyforge::make_initial_state(*eraPackage);
        worldState.position.scene = storyforge::SceneId{"scene.hall"};
        worldState.completed_events.insert(storyforge::EventId{"event.accident.resolved"});
        if (!std::holds_alternative<storyforge::RuntimeError>(storyforge::execute_agent_requests(
                *eraPackage, worldState, {{storyforge::AgentActionKind::authored,
                    "action.followup", {}}}))) {
            next.followupSeen = true;
            if (state.clueFound) next.clueShared = true;
            const std::string npc = eraPackage->agent_npcs.empty() ? "一位同学"
                : eraPackage->agent_npcs.front().name;
            exploration = state.followupSeen && state.clueFound
                ? npc + "接过你后来找到的借阅线索，仔细核对了铜鸟的编号。你们决定继续追查。"
                : npc + (state.accidentOutcome == "cast" ? "说：“那天多亏你托住了铜鸟。”"
                : state.accidentOutcome == "help" ? "说：“你先顾着大家，我记得。”"
                : state.accidentOutcome == "missed" ? "说：“你缺席那天，教室里险些出了事。”"
                : "说：“那天你先退开了。我想知道你后来有没有查过原因。”");
            if (!state.followupSeen && state.clueFound)
                exploration += "你拿出图书馆的借阅线索，对方立刻认真起来。";
        }
    }
    if (locationId == "great_hall" || locationId == "library"
        || locationId == "courtyard" || locationId == "hogsmeade")
        next.reputation = std::min(100, next.reputation + 1);
    const auto beat = advance(next);
    return {true, next, exploration + beat};
}

} // namespace arcane::application::world
