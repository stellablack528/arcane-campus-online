#include "application/story/StoryContentLoader.hpp"
#include "application/story/StoryContentValidator.hpp"
#include "application/story/StoryRuntimeService.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringConverter>
#include <QTextStream>
#include <QVector>

#include <iostream>

namespace story = arcane::application::story;

namespace {

struct Report {
    bool passed = true;
    QStringList lines;

    void check(bool condition, const QString& label, const QString& detail = {})
    {
        passed = passed && condition;
        lines.push_back(QStringLiteral("[%1] %2%3")
                            .arg(condition ? QStringLiteral("PASS") : QStringLiteral("FAIL"),
                                 label,
                                 detail.isEmpty() ? QString() : QStringLiteral(" | ") + detail));
    }
};

struct HouseCase {
    QString house;
    QString actionId;
    QString resultFlag;
    QString hatPhrase;
};

bool writeReport(const QString& path, const Report& report)
{
    if (path.isEmpty()) return true;
    const QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath())) return false;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;
    QTextStream output(&file);
    output.setEncoding(QStringConverter::Utf8);
    output << "Arcane Campus Online - Standalone Sorting Ceremony report\n";
    output << "=========================================================\n";
    for (const auto& line : report.lines) output << line << '\n';
    output << "---------------------------------------------------------\n";
    output << "OVERALL: " << (report.passed ? "PASS" : "FAIL") << '\n';
    output << "PLAYABLE FLOW: Black Lake -> Entrance Hall -> Sorting Hat -> House Table\n";
    output << "INTEGRATION STATUS: Registered in the built-in campaign and hands off to free exploration.\n";
    return true;
}

story::StoryRuntimeState baseState(const story::ChapterDefinition& chapter,
                                   const QString& house)
{
    story::StoryRuntimeState state;
    state.characterId = 42;
    state.chapterId = chapter.chapterId;
    state.contentVersion = chapter.contentVersion;
    state.currentSceneId = chapter.entrySceneId;
    state.completedChapters.insert(QStringLiteral("journey_to_hogwarts"));
    state.unlockedChapters.insert(QStringLiteral("sorting_ceremony"));
    state.characterAttributes.insert(QStringLiteral("house"), house);
    state.npcAffinities.insert(QStringLiteral("rowan_vale"), 2);
    state.npcMemoryFacts[QStringLiteral("rowan_vale")].insert(
        QStringLiteral("met_on_platform"));
    state.npcMemoryFacts[QStringLiteral("rowan_vale")].insert(
        QStringLiteral("helped_with_trunk"));
    return state;
}

void acceptAllCommits(story::StoryRuntimeService& runtime)
{
    runtime.setCommitter(
        [](const story::ChapterDefinition&, const story::StoryRuntimeState&,
           const story::StoryExecutionResult&, QString&) { return true; });
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    const QStringList arguments = app.arguments();
    const qsizetype reportIndex = arguments.indexOf(QStringLiteral("--report"));
    const QString reportPath = reportIndex >= 0 && reportIndex + 1 < arguments.size()
        ? arguments.at(reportIndex + 1) : QString();
    Report report;

    const story::StoryContentLoader loader;
    const auto loaded = loader.loadFile(
        QStringLiteral(":/story/chapters/sorting_ceremony.json"));
    report.check(loaded.success(),
                 QStringLiteral("Sorting Ceremony content loads"),
                 loaded.errors.join(QStringLiteral("; ")));
    if (!loaded.chapter) {
        (void)writeReport(reportPath, report);
        return 1;
    }

    const story::StoryContentValidator validator;
    const auto validation = validator.validate(*loaded.chapter);
    report.check(validation.isValid(),
                 QStringLiteral("Chapter IDs, bilingual text and scene graph validate"),
                 validation.errors.join(QStringLiteral("; ")));

    story::StoryRuntimeState missingRequirement = baseState(
        *loaded.chapter, QStringLiteral("Gryffindor"));
    missingRequirement.completedChapters.clear();
    story::StoryRuntimeService blockedRuntime;
    acceptAllCommits(blockedRuntime);
    report.check(!blockedRuntime.start(*loaded.chapter, missingRequirement),
                 QStringLiteral("Journey completion is required before Sorting"));

    const QVector<HouseCase> houseCases{
        {QStringLiteral("Gryffindor"), QStringLiteral("be_sorted_into_gryffindor"),
         QStringLiteral("sorted_into_gryffindor"), QStringLiteral("格兰芬多")},
        {QStringLiteral("Slytherin"), QStringLiteral("be_sorted_into_slytherin"),
         QStringLiteral("sorted_into_slytherin"), QStringLiteral("斯莱特林")},
        {QStringLiteral("Ravenclaw"), QStringLiteral("be_sorted_into_ravenclaw"),
         QStringLiteral("sorted_into_ravenclaw"), QStringLiteral("拉文克劳")},
        {QStringLiteral("Hufflepuff"), QStringLiteral("be_sorted_into_hufflepuff"),
         QStringLiteral("sorted_into_hufflepuff"), QStringLiteral("赫奇帕奇")}
    };

    for (const auto& houseCase : houseCases) {
        story::StoryRuntimeService runtime;
        acceptAllCommits(runtime);
        QStringList narrative;
        QObject::connect(&runtime, &story::StoryRuntimeService::narrativeProduced,
                         [&](const QString&, const QString& text) {
                             narrative.push_back(text);
                         });

        bool routeAccepted = runtime.start(*loaded.chapter,
                                           baseState(*loaded.chapter, houseCase.house));
        const bool rowanRememberedPlayer = narrative.join(QLatin1Char('\n')).contains(
            QStringLiteral("我替你留了位置"));
        routeAccepted = routeAccepted
            && runtime.submitAction(QStringLiteral("cross_the_black_lake"))
            && runtime.submitAction(QStringLiteral("enter_the_great_hall"));
        const bool onlyMatchingHouseChoice = runtime.visibleActionIds()
            == QStringList{houseCase.actionId};
        routeAccepted = routeAccepted
            && runtime.submitAction(houseCase.actionId)
            && runtime.submitAction(QStringLiteral("take_your_place_at_the_table"));

        const auto* finalState = runtime.state();
        const QString allNarrative = narrative.join(QLatin1Char('\n'));
        report.check(routeAccepted && onlyMatchingHouseChoice && finalState
                         && finalState->chapterCompleted
                         && finalState->currentSceneId == QLatin1String("house_tables")
                         && finalState->flags.contains(houseCase.resultFlag)
                         && finalState->flags.contains(QStringLiteral("house_table_reached"))
                         && finalState->flags.contains(QStringLiteral("main_story_complete"))
                         && finalState->flags.contains(
                             QStringLiteral("open_world_handoff_ready"))
                         && finalState->unlockedChapters.contains(
                             QStringLiteral("first_night_at_hogwarts"))
                         && allNarrative.contains(houseCase.hatPhrase),
                     QStringLiteral("%1 receives its own Hat verdict and completes")
                         .arg(houseCase.house));
        if (houseCase.house == QLatin1String("Gryffindor")) {
            report.check(rowanRememberedPlayer
                             && finalState
                             && finalState->npcMemoryFacts
                                    .value(QStringLiteral("rowan_vale"))
                                    .contains(QStringLiteral("crossed_black_lake_together"))
                             && finalState->npcMemoryFacts
                                    .value(QStringLiteral("rowan_vale"))
                                    .contains(QStringLiteral("saw_sorting")),
                         QStringLiteral("Rowan's earlier relationship changes the lake scene and persists"));

            const story::StoryRuntimeState completedState = *finalState;
            story::StoryRuntimeService completedResumeRuntime;
            acceptAllCommits(completedResumeRuntime);
            report.check(completedResumeRuntime.start(*loaded.chapter, completedState)
                             && completedResumeRuntime.state()
                             && completedResumeRuntime.state()->chapterCompleted
                             && completedResumeRuntime.state()->flags.contains(
                                 QStringLiteral("open_world_handoff_ready"))
                             && completedResumeRuntime.visibleActionIds().isEmpty(),
                         QStringLiteral("Completed main-story state restores without reopening guided choices"));
        }
    }

    story::StoryRuntimeState resumedState = baseState(
        *loaded.chapter, QStringLiteral("Ravenclaw"));
    resumedState.currentSceneId = QStringLiteral("entrance_hall");
    resumedState.visitedScenes.insert(QStringLiteral("black_lake_crossing"));
    story::StoryRuntimeService resumedRuntime;
    acceptAllCommits(resumedRuntime);
    QString resumedNarrative;
    QObject::connect(&resumedRuntime, &story::StoryRuntimeService::narrativeProduced,
                     [&](const QString&, const QString& text) {
                         resumedNarrative += text;
                     });
    report.check(resumedRuntime.start(*loaded.chapter, resumedState)
                     && resumedRuntime.state()
                     && resumedRuntime.state()->currentSceneId
                            == QLatin1String("entrance_hall")
                     && resumedNarrative.contains(QStringLiteral("那顶帽子也像我的箱子")),
                 QStringLiteral("A mid-chapter resume restores scene and Rowan memory dialogue"));

    story::StoryRuntimeState strangerState = baseState(
        *loaded.chapter, QStringLiteral("Hufflepuff"));
    strangerState.npcAffinities.clear();
    strangerState.npcMemoryFacts.clear();
    story::StoryRuntimeService strangerRuntime;
    acceptAllCommits(strangerRuntime);
    QString strangerNarrative;
    QObject::connect(&strangerRuntime, &story::StoryRuntimeService::narrativeProduced,
                     [&](const QString&, const QString& text) {
                         strangerNarrative += text;
                     });
    report.check(strangerRuntime.start(*loaded.chapter, strangerState)
                     && strangerNarrative.contains(QStringLiteral("现在才正式认识"))
                     && !strangerNarrative.contains(QStringLiteral("我替你留了位置")),
                 QStringLiteral("A player Rowan does not remember receives the first-meeting branch"));

    const bool reportWritten = writeReport(reportPath, report);
    for (const auto& line : report.lines) std::cout << line.toStdString() << '\n';
    return report.passed && reportWritten ? 0 : 1;
}
