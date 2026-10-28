#include "application/story/StoryRuntimeService.hpp"
#include "application/story/campaign/StoryCampaignService.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringConverter>
#include <QTextStream>

#include <iostream>

namespace story = arcane::application::story;
namespace campaign = arcane::application::story::campaign;

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

bool writeReport(const QString& path, const Report& report)
{
    if (path.isEmpty()) return true;
    const QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath())) return false;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;
    QTextStream output(&file);
    output.setEncoding(QStringConverter::Utf8);
    output << "Arcane Campus Online - Complete first-year prologue report\n";
    output << "==========================================================\n";
    for (const auto& line : report.lines) output << line << '\n';
    output << "----------------------------------------------------------\n";
    output << "OVERALL: " << (report.passed ? "PASS" : "FAIL") << '\n';
    output << "PLAYABLE FLOW: Diagon Alley -> King's Cross -> Platform 9 3/4 -> "
              "Hogwarts Express -> Hogsmeade -> Black Lake -> Sorting -> Charms -> Free Exploration\n";
    return true;
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

    campaign::StoryCampaignService campaign;
    QStringList loadErrors;
    report.check(campaign.loadBuiltIn(loadErrors),
                 QStringLiteral("Built-in campaign loads and validates"),
                 loadErrors.join(QStringLiteral("; ")));
    report.check(campaign.chapterIds()
                     == QStringList{QStringLiteral("diagon_alley_shopping"),
                                    QStringLiteral("first_charms_lesson"),
                                    QStringLiteral("journey_to_hogwarts"),
                                    QStringLiteral("sorting_ceremony"),
                                    QStringLiteral("turning_pages")},
                 QStringLiteral("Campaign catalog exposes all five playable chapters"));

    const auto diagonAlley = campaign.chapter(QStringLiteral("diagon_alley_shopping"));
    const auto journey = campaign.chapter(QStringLiteral("journey_to_hogwarts"));
    const auto sorting = campaign.chapter(QStringLiteral("sorting_ceremony"));
    const auto charms = campaign.chapter(QStringLiteral("first_charms_lesson"));
    report.check(diagonAlley.has_value() && journey.has_value() && sorting.has_value()
                     && charms.has_value(),
                 QStringLiteral("All chapter definitions are available to the runtime"));
    if (!diagonAlley || !journey || !sorting || !charms) return 1;

    story::StoryRuntimeState shoppingComplete;
    shoppingComplete.characterId = 42;
    shoppingComplete.chapterId = diagonAlley->chapterId;
    shoppingComplete.contentVersion = diagonAlley->contentVersion;
    shoppingComplete.currentSceneId = QStringLiteral("street");
    shoppingComplete.gold = 10;
    shoppingComplete.flags = {
        QStringLiteral("wand"), QStringLiteral("books"), QStringLiteral("robes"),
        QStringLiteral("potions"), QStringLiteral("pet"),
        QStringLiteral("shopping_complete"), QStringLiteral("visited_street")};
    shoppingComplete.chapterCompleted = true;
    shoppingComplete.completedChapters.insert(diagonAlley->chapterId);
    shoppingComplete.inventory.insert(8, 1);

    story::StoryRuntimeService departureRuntime;
    acceptAllCommits(departureRuntime);
    QString transitionTarget;
    QObject::connect(&departureRuntime,
                     &story::StoryRuntimeService::chapterTransitionRequested,
                     [&](const QString& chapterId) { transitionTarget = chapterId; });
    const bool departureStarted = departureRuntime.start(*diagonAlley, shoppingComplete);
    const bool departureAccepted = departureRuntime.submitText(
        QStringLiteral("前往国王十字车站"));
    report.check(departureStarted && departureAccepted
                     && transitionTarget == QLatin1String("journey_to_hogwarts")
                     && departureRuntime.state()
                     && departureRuntime.state()->unlockedChapters.contains(transitionTarget),
                 QStringLiteral("Completed shopping can explicitly transition to King's Cross"));

    story::StoryRuntimeState journeyState;
    journeyState.characterId = 42;
    journeyState.chapterId = journey->chapterId;
    journeyState.contentVersion = journey->contentVersion;
    journeyState.currentSceneId = journey->entrySceneId;
    journeyState.gold = 10;
    journeyState.inventory = shoppingComplete.inventory;
    journeyState.characterAttributes.insert(QStringLiteral("house"),
                                             QStringLiteral("Ravenclaw"));
    journeyState.completedChapters = shoppingComplete.completedChapters;
    journeyState.unlockedChapters = departureRuntime.state()
        ? departureRuntime.state()->unlockedChapters : QSet<QString>{};

    story::StoryRuntimeService journeyRuntime;
    acceptAllCommits(journeyRuntime);
    bool petNarrationSeen = false;
    QObject::connect(&journeyRuntime, &story::StoryRuntimeService::narrativeProduced,
                     [&](const QString&, const QString& text) {
                         petNarrationSeen = petNarrationSeen
                             || text.contains(QStringLiteral("猫头鹰在笼中"));
                     });
    bool routeAccepted = journeyRuntime.start(*journey, journeyState);
    routeAccepted = routeAccepted
        && journeyRuntime.submitText(QStringLiteral("跟着那家人"))
        && journeyRuntime.submitText(QStringLiteral("帮忙抬箱子"));
    const auto* platformState = journeyRuntime.state();
    report.check(routeAccepted && platformState
                     && platformState->currentSceneId
                            == QLatin1String("platform_nine_three_quarters")
                     && platformState->npcAffinities.value(QStringLiteral("rowan_vale")) == 1
                     && platformState->npcMemoryFacts.value(QStringLiteral("rowan_vale"))
                            .contains(QStringLiteral("helped_with_trunk")),
                 QStringLiteral("Station choices reach the platform and establish NPC memory"));

    story::StoryRuntimeState resumedState = platformState ? *platformState : journeyState;
    story::StoryRuntimeService resumedRuntime;
    acceptAllCommits(resumedRuntime);
    bool warmResumeSeen = false;
    QString sortingTransitionTarget;
    QObject::connect(&resumedRuntime, &story::StoryRuntimeService::narrativeProduced,
                     [&](const QString&, const QString& text) {
                         warmResumeSeen = warmResumeSeen
                             || text.contains(QStringLiteral("箱子已经承认我们赢了"));
                         petNarrationSeen = petNarrationSeen
                             || text.contains(QStringLiteral("猫头鹰在笼中"));
                     });
    QObject::connect(&resumedRuntime,
                     &story::StoryRuntimeService::chapterTransitionRequested,
                     [&](const QString& chapterId) { sortingTransitionTarget = chapterId; });
    routeAccepted = resumedRuntime.start(*journey, resumedState)
        && resumedRuntime.submitText(QStringLiteral("上车"))
        && resumedRuntime.submitText(QStringLiteral("分享糖果"))
        && resumedRuntime.submitText(QStringLiteral("准备下车"))
        && resumedRuntime.submitText(QStringLiteral("跟随新生"));
    const auto* finalState = resumedRuntime.state();
    report.check(warmResumeSeen,
                 QStringLiteral("Resume restores Rowan's memory and changes the greeting"));
    report.check(petNarrationSeen,
                 QStringLiteral("The train scene reflects the companion purchased in Diagon Alley"));
    report.check(routeAccepted && finalState && finalState->chapterCompleted
                     && finalState->currentSceneId == QLatin1String("hogsmeade_station")
                     && finalState->flags.contains(QStringLiteral("reached_hogwarts"))
                     && finalState->unlockedChapters.contains(QStringLiteral("sorting_ceremony"))
                     && sortingTransitionTarget == QLatin1String("sorting_ceremony"),
                 QStringLiteral("The complete journey reaches Hogwarts and transitions to Sorting"));

    story::StoryRuntimeState sortingState;
    sortingState.characterId = 42;
    sortingState.chapterId = sorting->chapterId;
    sortingState.contentVersion = sorting->contentVersion;
    sortingState.currentSceneId = sorting->entrySceneId;
    sortingState.gold = finalState ? finalState->gold : 10;
    sortingState.inventory = finalState ? finalState->inventory : shoppingComplete.inventory;
    sortingState.characterAttributes = finalState
        ? finalState->characterAttributes : journeyState.characterAttributes;
    sortingState.completedChapters = finalState
        ? finalState->completedChapters : QSet<QString>{QStringLiteral("journey_to_hogwarts")};
    sortingState.unlockedChapters = finalState
        ? finalState->unlockedChapters : QSet<QString>{QStringLiteral("sorting_ceremony")};
    sortingState.npcAffinities = finalState
        ? finalState->npcAffinities : QHash<QString, int>{};
    sortingState.npcMemoryFacts = finalState
        ? finalState->npcMemoryFacts : QHash<QString, QSet<QString>>{};

    story::StoryRuntimeService sortingRuntime;
    acceptAllCommits(sortingRuntime);
    bool mainStoryEndingSeen = false;
    QString completedChapter;
    QObject::connect(&sortingRuntime, &story::StoryRuntimeService::narrativeProduced,
                     [&](const QString&, const QString& text) {
                         mainStoryEndingSeen = mainStoryEndingSeen
                             || (text.contains(QStringLiteral("魔咒课"))
                                 && text.contains(QStringLiteral("自主探索")));
                     });
    QObject::connect(&sortingRuntime, &story::StoryRuntimeService::chapterCompleted,
                     [&](const QString& chapterId) { completedChapter = chapterId; });
    routeAccepted = sortingRuntime.start(*sorting, sortingState)
        && sortingRuntime.submitAction(QStringLiteral("cross_the_black_lake"))
        && sortingRuntime.submitAction(QStringLiteral("enter_the_great_hall"))
        && sortingRuntime.submitAction(QStringLiteral("be_sorted_into_ravenclaw"))
        && sortingRuntime.submitAction(QStringLiteral("take_your_place_at_the_table"));
    const auto* prologueFinalState = sortingRuntime.state();
    report.check(routeAccepted && prologueFinalState
                     && prologueFinalState->chapterCompleted
                     && prologueFinalState->currentSceneId == QLatin1String("house_tables")
                     && prologueFinalState->unlockedChapters.contains(
                         QStringLiteral("first_charms_lesson"))
                     && !prologueFinalState->flags.contains(QStringLiteral("main_story_complete"))
                     && completedChapter == QLatin1String("sorting_ceremony"),
                 QStringLiteral("Sorting leads into the first Charms lesson"));
    report.check(mainStoryEndingSeen,
                 QStringLiteral("Sorting narrative guides the player toward exploration"));

    story::StoryRuntimeState charmsState = prologueFinalState ? *prologueFinalState : sortingState;
    charmsState.chapterId = charms->chapterId;
    charmsState.contentVersion = charms->contentVersion;
    charmsState.currentSceneId = charms->entrySceneId;
    charmsState.chapterCompleted = false;
    story::StoryRuntimeService charmsRuntime;
    acceptAllCommits(charmsRuntime);
    const bool classStarted = charmsRuntime.start(*charms, charmsState);
    const bool castBeforePractice = charmsRuntime.submitAction(QStringLiteral("cast_levitation_charm"));
    const bool classCompleted = charmsRuntime.submitAction(QStringLiteral("practice_levitation"))
        && charmsRuntime.submitAction(QStringLiteral("cast_levitation_charm"));
    const auto* completedClass = charmsRuntime.state();
    report.check(classStarted && !castBeforePractice && classCompleted && completedClass
                     && completedClass->chapterCompleted
                     && completedClass->flags.contains(QStringLiteral("levitation_mastered"))
                     && completedClass->flags.contains(QStringLiteral("main_story_complete"))
                     && completedClass->flags.contains(QStringLiteral("open_world_handoff_ready")),
                 QStringLiteral("Practice and casting complete Charms and unlock exploration"));

    const bool reportWritten = writeReport(reportPath, report);
    for (const auto& line : report.lines) std::cout << line.toStdString() << '\n';
    return report.passed && reportWritten ? 0 : 1;
}
