#include "CharacterDAO.hpp"
#include "InventoryDAO.hpp"
#include "LocalSaveBootstrap.hpp"
#include "LocationDAO.hpp"
#include "StoryDAO.hpp"
#include "UserDAO.hpp"
#include "application/diagon_alley/DiagonAlleyCatalog.hpp"
#include "application/service/SessionService.hpp"
#include "application/story/StoryContentLoader.hpp"
#include "application/story/StoryRuntimeService.hpp"
#include "application/story/adapters/DiagonAlleyChapterAdapter.hpp"
#include "application/story/persistence/StoryPersistenceService.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>

#include <iostream>

namespace database = arcane::database;
namespace story = arcane::application::story;

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QDir().mkpath(QStringLiteral("test-results"));
    QTemporaryDir folder(
        QDir::current().filePath(QStringLiteral("test-results/local-save-XXXXXX")));
    if (!folder.isValid())
        return 1;
    const auto path =
        std::filesystem::path((folder.path() + QStringLiteral("/campus.sqlite")).toStdWString());
    story::StoryContentLoader loader;
    const auto loaded = loader.loadFile(QStringLiteral(":/story/chapters/turning_pages.json"));
    if (!loaded.chapter)
        return 12;
    const auto chapter = *loaded.chapter;
    arcane::application::diagon_alley::DiagonAlleyCatalog catalog;
    if (!catalog.load())
        return 17;
    const auto alleyChapter = story::adapters::DiagonAlleyChapterAdapter{}.build(catalog);
    std::uint64_t characterId = 0;
    std::string error;
    {
        auto connection = database::LocalSaveBootstrap::open(path, &error);
        if (!connection) {
            std::cerr << "local save " << path.string() << ": " << error << '\n';
            return 2;
        }
        const auto courtyard =
            connection->query("SELECT room_id FROM rooms WHERE room_name = 'Courtyard'");
        const auto wandItem = connection->query("SELECT item_id FROM items WHERE item_id = 1");
        if (!courtyard || courtyard->rows.empty() || !wandItem || wandItem->rows.empty())
            return 3;

        auto users = std::make_shared<database::UserDAO>(connection);
        auto characters = std::make_shared<database::CharacterDAO>(connection);
        auto inventory = std::make_shared<database::InventoryDAO>(connection);
        arcane::application::service::SessionService enrollment(users, characters, inventory);
        arcane::application::dto::EnrollmentRequestDTO request;
        request.username = "local_first_year";
        request.nickname = "Local First Year";
        request.house = "Ravenclaw";
        request.gender = "other";
        request.bloodStatus = "halfblood";
        if (!enrollment.registerStudent(request).success)
            return 4;
        const auto createdUser = users->getUserByName(request.username);
        if (!createdUser)
            return 5;
        const auto createdCharacter = characters->getCharacterByUserId(createdUser->userId);
        if (!createdCharacter)
            return 7;
        characterId = createdCharacter->characterId;

        database::StoryDAO progress(connection);
        database::CharacterWandRecord wand;
        wand.characterId = characterId;
        wand.woodId = "oak";
        wand.coreId = "phoenix";
        wand.woodName = "Oak";
        wand.coreName = "Phoenix feather";
        wand.lengthInches = 11.0;
        wand.flexibility = "supple";
        wand.handleStyle = "plain";
        wand.finish = "polished";
        wand.colorTone = "brown";
        if (!progress.upsertWand(wand) || !progress.markWandClaimed(characterId))
            return 8;
        if (!progress.upsertStoryState(
                {characterId, "prologue", "diagon_alley", "diagon_alley_arrival", ""}))
            return 9;
        if (!progress.upsertStoryState(
                {characterId, "diagon_alley_shopping", "street", "active", "wand"}))
            return 18;
        database::LocationDAO locations(connection);
        if (!locations.upsertLocation(characterId, 1, 0, 0, true))
            return 10;
        if (!connection->execute(
                "INSERT INTO character_story_states "
                "(character_id, chapter_id, scene_id, stage, flags, status, "
                "state_json) "
                "VALUES (?, 'first_charms_lesson', 'charms_classroom', "
                "'completed', '', "
                "'completed', ?)",
                {std::to_string(characterId),
                 R"({"flags":["levitation_mastered","main_story_complete","open_world_handoff_ready"]})"}))
            return 11;

        story::persistence::StoryPersistenceService persistence(connection);
        const auto alleyRestored = persistence.load(alleyChapter, characterId);
        if (!alleyRestored.success || !alleyRestored.found ||
            !alleyRestored.state.flags.contains(QStringLiteral("wand")))
            return 19;
        story::StoryRuntimeService alleyRuntime;
        alleyRuntime.setCommitter([&persistence](const story::ChapterDefinition &definition,
                                                 const story::StoryRuntimeState &state,
                                                 const story::StoryExecutionResult &result,
                                                 QString &commitError) {
            return persistence.commit(definition, state, result, commitError);
        });
        if (!alleyRuntime.start(alleyChapter, alleyRestored.state) ||
            !alleyRuntime.submitAction(QStringLiteral("enter_flourish_and_blotts")) ||
            !alleyRuntime.submitAction(QStringLiteral("buy_first_year_books"))) {
            std::cerr << connection->lastError() << '\n';
            return 20;
        }
        alleyRuntime.stop();

        const auto restored = persistence.load(chapter, characterId);
        if (!restored.success || restored.found ||
            !restored.state.flags.contains(QStringLiteral("levitation_mastered")))
            return 13;

        story::StoryRuntimeService runtime;
        runtime.setCommitter([&persistence](const story::ChapterDefinition &definition,
                                            const story::StoryRuntimeState &state,
                                            const story::StoryExecutionResult &result,
                                            QString &commitError) {
            return persistence.commit(definition, state, result, commitError);
        });
        if (!runtime.start(chapter, restored.state) ||
            !runtime.submitAction(QStringLiteral("hall_to_library")) ||
            !runtime.submitAction(QStringLiteral("inspect_turning_book")) ||
            !runtime.submitAction(QStringLiteral("lift_page_with_charm")) ||
            !runtime.submitAction(QStringLiteral("library_to_hall")) ||
            !runtime.submitAction(QStringLiteral("share_story_with_rowan"))) {
            std::cerr << connection->lastError() << '\n';
            return 14;
        }
        runtime.stop();
    }

    auto reopened = database::LocalSaveBootstrap::open(path, &error);
    if (!reopened)
        return 15;
    arcane::application::service::SessionService resumedSession(
        std::make_shared<database::UserDAO>(reopened),
        std::make_shared<database::CharacterDAO>(reopened),
        std::make_shared<database::InventoryDAO>(reopened));
    if (!resumedSession.enterCampus({"local_first_year", "Ravenclaw"}).success)
        return 21;
    story::persistence::StoryPersistenceService reopenedPersistence(reopened);
    const auto resumedAlley = reopenedPersistence.load(alleyChapter, characterId);
    const auto continued = reopenedPersistence.load(chapter, characterId);
    database::StoryDAO reopenedProgress(reopened);
    const auto latest = reopenedProgress.getStoryState(characterId);
    const auto events =
        reopened->query("SELECT event_id FROM story_event_log WHERE character_id = ?",
                        {std::to_string(characterId)});
    if (!resumedAlley.success || !resumedAlley.found ||
        resumedAlley.state.currentSceneId != QLatin1String("flourish_and_blotts") ||
        !resumedAlley.state.flags.contains(QStringLiteral("books")) ||
        resumedAlley.state.inventory.value(12) != 1 || resumedAlley.state.gold != 38 ||
        !continued.success || !continued.found || !continued.state.chapterCompleted ||
        !continued.state.npcMemoryFacts.value(QStringLiteral("rowan_vale"))
             .contains(QStringLiteral("shared_erins_story")) ||
        !events || events->rows.size() < 6 || !latest || latest->chapterId != "turning_pages")
        return 16;
    std::cout << "PASS: automatic local schema, character, wand, Diagon Alley purchase, "
                 "story transaction and restart\n";
    return 0;
}
