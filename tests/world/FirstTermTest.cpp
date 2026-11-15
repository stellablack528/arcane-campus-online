#include "application/world/FirstTermEngine.hpp"
#include "application/world/FirstTermStore.hpp"
#include "application/world/EraWorldPack.hpp"
#include "DBConnection.hpp"
#include <storyforge/model/story_types.hpp>

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>

#include <iostream>
#include <memory>

using namespace arcane::application::world;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    // 开场魔咒课已结束，第一学期从第一周周一午餐接续。
    FirstTermState state;
    if (state.day != 1 || state.phase != 2 || state.attended != 1 || state.subjects[0] != 5)
        return 1;

    int turns = 0;
    while (!state.complete && turns < 245) {
        const int subject = FirstTermEngine::subjectFor(state);
        TermAction action = TermAction::Explore;
        const char* place = "courtyard";
        if (state.phase == 0 || state.phase == 2 || state.phase == 4) {
            action = TermAction::Eat;
            place = "great_hall";
        } else if (subject >= 0) {
            action = TermAction::Attend;
            place = FirstTermEngine::classroomFor(subject);
        } else if (state.phase == 6) action = TermAction::Rest;
        const auto result = FirstTermEngine::apply(state, action, place);
        if (!result.accepted) { std::cerr << result.narration << '\n'; return 1; }
        state = result.next;
        ++turns;
    }
    if (!state.complete || turns != 243 || state.attended != 50 || state.meals != 104
        || state.missed != 0 || state.health == 0 || state.day != 36) {
        std::cerr << "schedule: day=" << state.day << " turns=" << turns
                  << " attended=" << state.attended << " meals=" << state.meals << '\n';
        return 1;
    }
    for (int score : state.subjects) if (score <= 5) return 1;

    // 错过饭点和缺课有明确后果，错误地点不能假装上课。
    FirstTermState neglected;
    auto result = FirstTermEngine::apply(neglected, TermAction::Skip, "courtyard");
    if (!result.accepted || result.next.health != 78 || result.next.phase != 3) return 1;
    neglected = result.next;
    result = FirstTermEngine::apply(neglected, TermAction::Attend, "great_hall");
    if (result.accepted || result.next.missed != 0) return 1;
    result = FirstTermEngine::apply(neglected, TermAction::Skip, "great_hall");
    if (!result.accepted || result.next.missed != 1 || result.next.reputation != -3) return 1;

    FirstTermState explorer;
    explorer.phase = 5;
    result = FirstTermEngine::apply(explorer, TermAction::Explore, "library");
    if (!result.accepted || result.next.reputation != 1) return 1;
    result = FirstTermEngine::apply(explorer, TermAction::Explore, "defense_classroom");
    if (!result.accepted || result.next.reputation != 0) return 1;

    FirstTermState weekend;
    weekend.day = 6;
    weekend.phase = 1;
    if (FirstTermEngine::subjectFor(weekend) != -1) return 1;
    weekend.day = 7;
    weekend.phase = 6;
    result = FirstTermEngine::apply(weekend, TermAction::Rest, "courtyard");
    if (!result.accepted || result.next.day != 8
        || result.narration.find("第二周") == std::string::npos) return 1;

    // 禁林要先到庭院；谨慎与深入的受伤程度不同，重伤后必须救治。
    FirstTermState danger;
    danger.phase = 5;
    danger.health = 30;
    if (FirstTermEngine::apply(danger, TermAction::EnterForest, "library").accepted) return 1;
    result = FirstTermEngine::apply(danger, TermAction::EnterForest, "courtyard");
    if (!result.accepted || !result.next.forestChoice) return 1;
    auto careful = FirstTermEngine::apply(result.next, TermAction::ForestCareful, "forbidden_forest");
    if (!careful.accepted || careful.next.health != 18 || careful.next.reputation != 2
        || careful.next.critical) {
        std::cerr << "forest careful: accepted=" << careful.accepted
                  << " hp=" << careful.next.health << " rep=" << careful.next.reputation
                  << " reason=" << careful.narration << '\n';
        return 1;
    }
    result = FirstTermEngine::apply(result.next, TermAction::ForestDeep, "forbidden_forest");
    if (!result.accepted || !result.next.critical || result.next.health != 0) {
        std::cerr << "forest deep: accepted=" << result.accepted
                  << " hp=" << result.next.health << " reason=" << result.narration << '\n';
        return 1;
    }
    if (FirstTermEngine::apply(result.next, TermAction::Rest, "courtyard").accepted) return 1;
    result = FirstTermEngine::apply(result.next, TermAction::Recover, "courtyard");
    if (!result.accepted || result.next.critical || result.next.health != 35) return 1;

    // 三个时代均能通过最新 TaleState JSON 加载器与语义校验。
    for (const auto* era : {"parents", "harry", "riddle"}) {
        QString error;
        const auto package = EraWorldPack::load(QString::fromLatin1(era), &error);
        if (!package) { std::cerr << "era load: " << era << " " << error.toStdString() << '\n'; return 1; }
        if (package->agent_npcs.size() < 2 || package->agent_actions.size() < 5) return 1;
    }
    const auto world = EraWorldPack::load(QStringLiteral("harry"));
    FirstTermState skilled;
    skilled.day = 5;
    skilled.phase = 1;
    skilled.subjects[0] = 10;
    result = FirstTermEngine::apply(skilled, TermAction::Attend, "charms_classroom", world.get());
    if (!result.accepted || !result.next.accidentPending || result.next.subjects[0] != 15) return 1;
    result = FirstTermEngine::apply(result.next, TermAction::AccidentCast, "charms_classroom", world.get());
    if (!result.accepted || result.next.accidentOutcome != "cast" || result.next.accidentPending) return 1;
    auto aftermath = result.next;
    aftermath.phase = 5;
    result = FirstTermEngine::apply(aftermath, TermAction::Explore, "library", world.get());
    if (!result.accepted || !result.next.clueFound) return 1;
    aftermath = result.next;
    aftermath.phase = 5;
    result = FirstTermEngine::apply(aftermath, TermAction::Explore, "great_hall", world.get());
    if (!result.accepted || !result.next.followupSeen || !result.next.clueShared
        || result.narration.find("借阅线索") == std::string::npos) return 1;

    // 先听同学回应、后找到线索时，仍可返回礼堂提供线索。
    aftermath = result.next;
    aftermath.clueFound = false;
    aftermath.clueShared = false;
    aftermath.followupSeen = false;
    aftermath.phase = 5;
    result = FirstTermEngine::apply(aftermath, TermAction::Explore, "great_hall", world.get());
    if (!result.accepted || !result.next.followupSeen || result.next.clueShared) return 1;
    aftermath = result.next;
    aftermath.phase = 5;
    result = FirstTermEngine::apply(aftermath, TermAction::Explore, "library", world.get());
    if (!result.accepted || !result.next.clueFound) return 1;
    aftermath = result.next;
    aftermath.phase = 5;
    result = FirstTermEngine::apply(aftermath, TermAction::Explore, "great_hall", world.get());
    if (!result.accepted || !result.next.clueShared
        || result.narration.find("继续追查") == std::string::npos) return 1;

    FirstTermState novice;
    novice.day = 5;
    novice.phase = 1;
    novice.subjects[0] = 5;
    result = FirstTermEngine::apply(novice, TermAction::Attend, "charms_classroom", world.get());
    if (!result.accepted || !result.next.accidentPending) return 1;
    if (FirstTermEngine::apply(result.next, TermAction::AccidentCast, "charms_classroom", world.get()).accepted) return 1;
    result = FirstTermEngine::apply(result.next, TermAction::AccidentHelp, "charms_classroom", world.get());
    if (!result.accepted || result.next.accidentOutcome != "help") return 1;
    result = FirstTermEngine::apply(novice, TermAction::Skip, "courtyard", world.get());
    if (!result.accepted || result.next.accidentOutcome != "missed") return 1;
    aftermath = result.next;
    aftermath.phase = 5;
    result = FirstTermEngine::apply(aftermath, TermAction::Explore, "great_hall", world.get());
    if (!result.accepted || !result.next.followupSeen || result.narration.find("缺席") == std::string::npos) return 1;

    // 三个时代分别完整走完首周；同一日程能承受施咒、求助、回避和缺席。
    for (const auto* era : {"parents", "harry", "riddle"}) {
        const auto eraWorld = EraWorldPack::load(QString::fromLatin1(era));
        for (const auto ending : {TermAction::AccidentCast, TermAction::AccidentHelp,
                                  TermAction::AccidentIgnore, TermAction::Skip}) {
            FirstTermState journey;
            journey.eraId = era;
            int steps = 0;
            while (journey.day <= 7 && steps++ < 60) {
                TermAction move = TermAction::Skip;
                std::string place = "courtyard";
                if (journey.accidentPending) {
                    move = ending;
                    if (ending == TermAction::Skip) return 1;
                    place = "charms_classroom";
                } else if (journey.phase == 0 || journey.phase == 2 || journey.phase == 4) {
                    move = TermAction::Eat;
                    place = "great_hall";
                } else if (FirstTermEngine::subjectFor(journey) >= 0) {
                    if (!(ending == TermAction::Skip && journey.day == 5 && journey.phase == 1)) {
                        move = TermAction::Attend;
                        place = FirstTermEngine::classroomFor(FirstTermEngine::subjectFor(journey));
                    }
                } else if (journey.phase == 5) {
                    move = TermAction::Explore;
                    place = journey.day == 4 ? "library"
                        : journey.day == 5 ? "great_hall" : "courtyard";
                } else if (journey.phase == 6) move = TermAction::Rest;
                const auto turn = FirstTermEngine::apply(journey, move, place, eraWorld.get());
                if (!turn.accepted) {
                    std::cerr << "first week " << era << " step " << steps << ": "
                              << turn.narration << '\n';
                    return 1;
                }
                journey = turn.next;
            }
            if (journey.day != 8 || !journey.clueFound || !journey.followupSeen || !journey.clueShared
                || journey.accidentPending || journey.meals != 20
                || journey.accidentOutcome != (ending == TermAction::Skip ? "missed"
                    : ending == TermAction::AccidentCast ? "cast"
                    : ending == TermAction::AccidentHelp ? "help" : "ignore")) {
                std::cerr << "first week outcome mismatch for " << era << '\n';
                return 1;
            }
        }
    }

    // 连续不吃饭最终需要救治，恢复后仍可继续游玩。
    FirstTermState exhausted;
    exhausted.health = 10;
    result = FirstTermEngine::apply(exhausted, TermAction::Skip, "courtyard");
    if (!result.accepted || !result.next.critical || result.next.health != 0) return 1;
    result = FirstTermEngine::apply(result.next, TermAction::Recover, "courtyard");
    if (!result.accepted || result.next.critical || result.next.health != 35) return 1;

    // 本地 SQLite 存档在重新打开连接后仍保持同一进度。
    QTemporaryDir temp(QDir::currentPath() + QStringLiteral("/term-test-XXXXXX"));
    if (!temp.isValid()) { std::cerr << "temporary directory unavailable\n"; return 1; }
    arcane::database::DBConfig config;
    config.driver = arcane::database::DatabaseDriverKind::Sqlite;
    config.database = (temp.path() + QStringLiteral("/first_term.sqlite")).toUtf8().toStdString();
    {
        auto connection = std::make_shared<arcane::database::DBConnection>(config);
        if (!connection->connect()) { std::cerr << "connect: " << connection->lastError() << '\n'; return 1; }
        FirstTermStore store(connection);
        state.eraId = "riddle";
        state.accidentOutcome = "help";
        state.clueFound = true;
        state.clueShared = true;
        if (!store.ready() || !store.save(7, state)) {
            std::cerr << "save: ready=" << store.ready() << " error=" << connection->lastError() << '\n';
            return 1;
        }
    }
    {
        auto connection = std::make_shared<arcane::database::DBConnection>(config);
        if (!connection->connect()) { std::cerr << "reconnect: " << connection->lastError() << '\n'; return 1; }
        FirstTermStore store(connection);
        const auto restored = store.load(7);
        if (!restored || restored->day != 36 || restored->attended != 50
            || restored->meals != 104 || restored->subjects != state.subjects) return 1;
        if (restored->eraId != "riddle" || restored->accidentOutcome != "help"
            || !restored->clueFound || !restored->clueShared) return 1;
    }
    std::cout << "Five-week term, consequences, forest, and save restore passed.\n";
    return 0;
}
