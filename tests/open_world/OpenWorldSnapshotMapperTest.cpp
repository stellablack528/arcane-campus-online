#include "application/open_world/mapping/OpenWorldSnapshotMapper.hpp"
#include "application/story/StoryTypes.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace open_world = arcane::application::open_world;
namespace story = arcane::application::story;

namespace {

struct Report {
    bool passed = true;
    std::vector<std::string> lines;

    void check(bool condition, const std::string& label)
    {
        passed = passed && condition;
        lines.push_back(std::string{"["} + (condition ? "PASS" : "FAIL") + "] " + label);
    }
};

story::StoryRuntimeState completedSortingState()
{
    story::StoryRuntimeState state;
    state.characterId = 42;
    state.chapterId = QStringLiteral("sorting_ceremony");
    state.contentVersion = 1;
    state.revision = 4;
    state.currentSceneId = QStringLiteral("house_tables");
    state.gold = 17;
    state.flags = {
        QStringLiteral("house_table_reached"),
        QStringLiteral("main_story_complete"),
        QStringLiteral("open_world_handoff_ready"),
        QStringLiteral("sorted_into_ravenclaw")};
    state.inventory.insert(101, 1);
    state.inventory.insert(205, 3);
    state.npcAffinities.insert(QStringLiteral("rowan_vale"), 4);
    state.npcAffinities.insert(QStringLiteral("ollivander"), 1);
    state.npcMemoryFacts[QStringLiteral("rowan_vale")].insert(
        QStringLiteral("met_on_platform"));
    state.npcMemoryFacts[QStringLiteral("rowan_vale")].insert(
        QStringLiteral("saw_sorting"));
    state.characterAttributes.insert(QStringLiteral("house"), QStringLiteral("Ravenclaw"));
    state.characterAttributes.insert(QStringLiteral("blood_status"), QStringLiteral("muggleborn"));
    state.characterAttributes.insert(QStringLiteral("wand_wood"), QStringLiteral("holly"));
    state.visitedScenes = {
        QStringLiteral("black_lake_crossing"),
        QStringLiteral("entrance_hall"),
        QStringLiteral("sorting_stool"),
        QStringLiteral("house_tables")};
    state.completedChapters = {
        QStringLiteral("diagon_alley"),
        QStringLiteral("journey_to_hogwarts"),
        QStringLiteral("sorting_ceremony")};
    state.unlockedChapters = {
        QStringLiteral("sorting_ceremony"),
        QStringLiteral("first_night_at_hogwarts")};
    state.chapterCompleted = true;
    return state;
}

bool writeReport(const std::string& path, const Report& report)
{
    if (path.empty()) return true;
    const std::filesystem::path reportPath{path};
    if (reportPath.has_parent_path())
        std::filesystem::create_directories(reportPath.parent_path());
    std::ofstream output(reportPath, std::ios::trunc);
    if (!output) return false;
    output << "Arcane Campus Online - Open World Snapshot Mapper report\n";
    output << "=======================================================\n";
    for (const auto& line : report.lines) output << line << '\n';
    output << "-------------------------------------------------------\n";
    output << "OVERALL: " << (report.passed ? "PASS" : "FAIL") << '\n';
    output << "SCOPE: Read-only Hogwarts state snapshot; no TaleState, network, or database writes.\n";
    return output.good();
}

} // namespace

int main(int argc, char* argv[])
{
    std::string reportPath;
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string{argv[i]} == "--report") reportPath = argv[++i];
    }

    Report report;
    const story::StoryRuntimeState source = completedSortingState();
    const story::StoryRuntimeState original = source;
    const auto mapped = open_world::OpenWorldSnapshotMapper::map(source);
    report.check(mapped.success(), "Completed Sorting state maps successfully");

    if (mapped.snapshot) {
        const auto& snapshot = *mapped.snapshot;
        report.check(snapshot.characterId == 42
                         && snapshot.chapterId == "sorting_ceremony"
                         && snapshot.currentSceneId == "house_tables"
                         && snapshot.acceptedStoryTurns == 4,
                     "Identity, position, and accepted turn count are preserved");
        report.check(snapshot.house == "Ravenclaw"
                         && snapshot.characterAttributes.at("blood_status") == "muggleborn"
                         && snapshot.characterAttributes.at("wand_wood") == "holly",
                     "House and character attributes are preserved");
        report.check(snapshot.gold == 17
                         && snapshot.inventory.at(101) == 1
                         && snapshot.inventory.at(205) == 3,
                     "Gold and inventory are preserved");
        report.check(snapshot.npcRelationships.at("rowan_vale") == 4
                         && snapshot.npcMemoryFacts.at("rowan_vale").contains("met_on_platform")
                         && snapshot.npcMemoryFacts.at("rowan_vale").contains("saw_sorting"),
                     "NPC relationships and memory facts are preserved");
        report.check(snapshot.completedChapters.contains("journey_to_hogwarts")
                         && snapshot.completedChapters.contains("sorting_ceremony")
                         && snapshot.unlockedChapters.contains("first_night_at_hogwarts")
                         && snapshot.visitedScenes.contains("black_lake_crossing"),
                     "Chapter anchors and visited scenes are preserved");
        report.check(snapshot.chapterCompleted && snapshot.mainStoryComplete
                         && snapshot.openWorldHandoffReady
                         && snapshot.readyForOpenWorld(),
                     "Main-story completion produces a ready open-world handoff");
    }

    report.check(source.characterId == original.characterId
                     && source.revision == original.revision
                     && source.flags == original.flags
                     && source.inventory == original.inventory
                     && source.npcAffinities == original.npcAffinities
                     && source.npcMemoryFacts == original.npcMemoryFacts,
                 "Mapping does not mutate the authoritative source state");

    story::StoryRuntimeState guidedState = completedSortingState();
    guidedState.flags.remove(QStringLiteral("main_story_complete"));
    guidedState.flags.remove(QStringLiteral("open_world_handoff_ready"));
    const auto guided = open_world::OpenWorldSnapshotMapper::map(guidedState);
    report.check(guided.success() && guided.snapshot
                     && !guided.snapshot->readyForOpenWorld(),
                 "A valid guided-story state maps without granting open-world access");

    story::StoryRuntimeState invalidState = completedSortingState();
    invalidState.characterId = 0;
    const auto invalid = open_world::OpenWorldSnapshotMapper::map(invalidState);
    report.check(!invalid.success() && !invalid.error.empty(),
                 "Missing authoritative identity is rejected with an error");

    story::StoryRuntimeState inconsistentState = completedSortingState();
    inconsistentState.flags.remove(QStringLiteral("main_story_complete"));
    const auto inconsistent = open_world::OpenWorldSnapshotMapper::map(inconsistentState);
    report.check(!inconsistent.success() && !inconsistent.error.empty(),
                 "An inconsistent open-world handoff is rejected");

    for (const auto& line : report.lines) std::cout << line << '\n';
    const bool reportWritten = writeReport(reportPath, report);
    return report.passed && reportWritten ? 0 : 1;
}
