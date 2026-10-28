#include "application/story/StoryContentLoader.hpp"
#include "application/story/StoryContentValidator.hpp"
#include "application/story/StoryRuntimeService.hpp"

#include <QCoreApplication>
#include <iostream>

namespace story = arcane::application::story;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    story::StoryContentLoader loader;
    const auto loaded = loader.loadFile(QStringLiteral(":/story/chapters/turning_pages.json"));
    if (!loaded.chapter || !story::StoryContentValidator{}.validate(*loaded.chapter).isValid())
        return 1;
    const auto& chapter = *loaded.chapter;
    for (const auto& ending : {QStringLiteral("share_story_with_rowan"),
                               QStringLiteral("ask_professor_to_archive"),
                               QStringLiteral("keep_story_private")}) {
        story::StoryRuntimeState state;
        state.characterId = 42;
        state.chapterId = chapter.chapterId;
        state.contentVersion = chapter.contentVersion;
        state.currentSceneId = chapter.entrySceneId;
        state.completedChapters.insert(QStringLiteral("first_charms_lesson"));
        state.flags.insert(QStringLiteral("main_story_complete"));
        state.flags.insert(QStringLiteral("open_world_handoff_ready"));
        story::StoryRuntimeService runtime;
        runtime.setCommitter([](const story::ChapterDefinition&, const story::StoryRuntimeState&,
                                const story::StoryExecutionResult&, QString&) { return true; });
        if (!runtime.start(chapter, state)) return 2;
        if (!runtime.submitAction(QStringLiteral("hall_to_courtyard"))
            || !runtime.submitAction(QStringLiteral("read_fountain_inscription"))
            || !runtime.submitAction(QStringLiteral("courtyard_to_library"))
            || !runtime.submitAction(QStringLiteral("inspect_turning_book"))) return 3;
        if (runtime.visibleActionIds().contains(QStringLiteral("lift_page_with_charm"))) return 4;
        if (!runtime.submitAction(QStringLiteral("ask_mira_to_open_page"))) return 5;
        if (ending == QLatin1String("share_story_with_rowan")
            && !runtime.submitAction(QStringLiteral("library_to_hall"))) return 5;
        if (ending == QLatin1String("ask_professor_to_archive")
            && !runtime.submitAction(QStringLiteral("library_to_courtyard"))) return 5;
        if (!runtime.submitAction(ending)) return 5;
        const auto* result = runtime.state();
        if (!result || !result->chapterCompleted || !result->flags.contains(QStringLiteral("page_read")))
            return 6;
        if (ending == QLatin1String("share_story_with_rowan")
            && !result->npcMemoryFacts.value(QStringLiteral("rowan_vale"))
                    .contains(QStringLiteral("shared_erins_story"))) return 7;
        if (ending == QLatin1String("ask_professor_to_archive")
            && !result->npcMemoryFacts.value(QStringLiteral("prof_flitwick"))
                    .contains(QStringLiteral("trusted_with_erins_story"))) return 8;
        if (ending == QLatin1String("keep_story_private")
            && !result->npcMemoryFacts.value(QStringLiteral("librarian_mira"))
                    .contains(QStringLiteral("respected_erins_privacy"))) return 9;
    }
    story::StoryRuntimeState magicState;
    magicState.characterId = 42;
    magicState.chapterId = chapter.chapterId;
    magicState.contentVersion = chapter.contentVersion;
    magicState.currentSceneId = chapter.entrySceneId;
    magicState.completedChapters.insert(QStringLiteral("first_charms_lesson"));
    magicState.flags.insert(QStringLiteral("levitation_mastered"));
    story::StoryRuntimeService magic;
    magic.setCommitter([](const story::ChapterDefinition&, const story::StoryRuntimeState&,
                          const story::StoryExecutionResult&, QString&) { return true; });
    if (!magic.start(chapter, magicState)
        || !magic.submitAction(QStringLiteral("hall_to_library"))
        || !magic.submitAction(QStringLiteral("inspect_turning_book"))
        || !magic.submitAction(QStringLiteral("lift_page_with_charm"))
        || !magic.state()->flags.contains(QStringLiteral("page_lifted_safely"))) return 10;
    std::cout << "PASS: three remembered endings, nonlinear travel, learned spell and nonmagic route\n";
    return 0;
}
