#pragma once

#include <array>
#include <string>

namespace storyforge { struct StoryPackage; }

namespace arcane::application::world {

enum class TermAction { Eat, Attend, Explore, Rest, Skip, EnterForest, ForestCareful, ForestDeep, ForestRetreat, Recover,
                        AccidentCast, AccidentHelp, AccidentIgnore };

struct FirstTermState {
    int day = 1;                 // 1..35, five seven-day weeks
    int phase = 2;               // 开场魔咒课后从午餐开始
    int health = 90;             // 0..100
    int reputation = 0;
    std::array<int, 4> subjects{5, 0, 0, 0}; // 开场魔咒课已完成
    int attended = 1;
    int missed = 0;
    int meals = 0;
    bool forestChoice = false;
    bool critical = false;
    bool complete = false;
    std::string eraId = "harry";
    bool accidentPending = false;
    std::string accidentOutcome;
    bool clueFound = false;
    bool followupSeen = false;
    bool clueShared = false;
};

struct TermOutcome {
    bool accepted = false;
    FirstTermState next;
    std::string narration;
};

class FirstTermEngine final {
public:
    [[nodiscard]] static int week(const FirstTermState& state) noexcept;
    [[nodiscard]] static int weekday(const FirstTermState& state) noexcept;
    [[nodiscard]] static int subjectFor(const FirstTermState& state) noexcept;
    [[nodiscard]] static const char* subjectName(int subject) noexcept;
    [[nodiscard]] static const char* classroomFor(int subject) noexcept;
    [[nodiscard]] static const char* phaseName(const FirstTermState& state) noexcept;
    [[nodiscard]] static TermOutcome apply(const FirstTermState& state, TermAction action,
                                           const std::string& locationId,
                                           const storyforge::StoryPackage* eraPackage = nullptr);
};

} // namespace arcane::application::world
