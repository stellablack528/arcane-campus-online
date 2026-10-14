#include "application/story/persistence/StoryPersistenceService.hpp"

#include "DBConnection.hpp"
#include "IDatabaseDriver.hpp"

#include <QCoreApplication>

#include <iostream>
#include <memory>

namespace database = arcane::database;
namespace story = arcane::application::story;
namespace persistence = arcane::application::story::persistence;

namespace {

bool expect(bool condition, const char* message)
{
    if (condition) return true;
    std::cerr << "FAILED: " << message << '\n';
    return false;
}

class StoryDatabaseDriver final : public database::IDatabaseDriver {
public:
    bool connect(const database::DBConfig&) override
    {
        connected = true;
        return true;
    }

    void disconnect() noexcept override
    {
        connected = false;
        transactionActive = false;
    }

    bool isConnected() const noexcept override { return connected; }

    bool execute(const std::string& sql, const database::QueryParams& params) override
    {
        if (!connected) return fail("not connected");
        if (sql.find("UPDATE inventories SET quantity") != std::string::npos) {
            inventoryQuantity = static_cast<std::uint32_t>(std::stoul(params.at(0)));
            return true;
        }
        if (sql.find("DELETE FROM inventories") != std::string::npos) {
            inventoryQuantity = 0;
            return true;
        }
        if (sql.find("INSERT INTO inventories") != std::string::npos) {
            inventoryQuantity = static_cast<std::uint32_t>(std::stoul(params.at(2)));
            return true;
        }
        if (sql.find("UPDATE characters SET gold") != std::string::npos) {
            gold = std::stoull(params.at(0));
            return true;
        }
        if (sql.find("INSERT INTO npc_memories") != std::string::npos) {
            affinity = std::stoi(params.at(2));
            memoryFlags = params.at(3);
            memoryJson = params.at(4);
            return true;
        }
        if (sql.find("UPDATE character_story_states SET content_version")
            != std::string::npos) {
            contentVersion = std::stoi(params.at(0));
            sceneId = params.at(1);
            status = params.at(4);
            stateJson = params.at(5);
            revision = std::stoull(params.at(6));
            return true;
        }
        if (sql.find("INSERT INTO character_story_states") != std::string::npos) {
            progressExists = true;
            contentVersion = std::stoi(params.at(2));
            sceneId = params.at(3);
            status = params.at(6);
            stateJson = params.at(7);
            revision = std::stoull(params.at(8));
            return true;
        }
        if (sql.find("INSERT INTO story_event_log") != std::string::npos) {
            if (failEventInsert) return fail("simulated event log failure");
            ++eventCount;
            return true;
        }
        if (sql.find("UPDATE character_story_states SET status = 'paused'")
            != std::string::npos) {
            ++pauseOtherChapterCount;
            return true;
        }
        return true;
    }

    std::optional<database::QueryResult> query(
        const std::string& sql, const database::QueryParams&) override
    {
        if (!connected) return std::nullopt;
        database::QueryResult result;
        if (sql.find("FROM characters") != std::string::npos) {
            result.rows.push_back({
                {"character_id", "42"}, {"gold", std::to_string(gold)},
                {"house", "Ravenclaw"}, {"blood_status", "halfblood"},
            });
        } else if (sql.find("FROM inventories") != std::string::npos) {
            if (inventoryQuantity > 0) {
                result.rows.push_back({
                    {"inventory_id", "100"}, {"item_id", "7"},
                    {"quantity", std::to_string(inventoryQuantity)},
                });
            }
        } else if (sql.find("FROM npc_memories") != std::string::npos) {
            result.rows.push_back({
                {"npc_id", "owl_keeper"}, {"affinity", std::to_string(affinity)},
                {"memory_flags", memoryFlags}, {"memory_json", memoryJson},
            });
        } else if (sql.find("SELECT chapter_id, status, state_json")
                   != std::string::npos) {
            result.rows.push_back({
                {"chapter_id", "earlier_chapter"}, {"status", "completed"},
                {"state_json", R"({"unlocked_chapters":["side_story"]})"},
            });
        } else if (sql.find("FROM character_wands") != std::string::npos) {
            result.rows.push_back({{"wood_id", "oak"}, {"core_id", "phoenix"}});
        } else if (sql.find("FROM character_story_states") != std::string::npos
                   && progressExists) {
            result.rows.push_back({
                {"chapter_id", "persistence_test"},
                {"content_version", std::to_string(contentVersion)},
                {"scene_id", sceneId}, {"status", status}, {"flags", "legacy"},
                {"state_json", stateJson}, {"revision", std::to_string(revision)},
            });
        }
        return result;
    }

    bool beginTransaction() override
    {
        if (!connected || transactionActive) return false;
        transactionActive = true;
        snapshot = {gold, inventoryQuantity, affinity, memoryFlags, memoryJson,
                    contentVersion, revision, sceneId, status, stateJson,
                    progressExists, eventCount, pauseOtherChapterCount};
        ++beginCount;
        return true;
    }

    bool commitTransaction() override
    {
        if (!transactionActive) return false;
        transactionActive = false;
        ++commitCount;
        return true;
    }

    bool rollbackTransaction() override
    {
        if (!transactionActive) return false;
        gold = snapshot.gold;
        inventoryQuantity = snapshot.inventoryQuantity;
        affinity = snapshot.affinity;
        memoryFlags = snapshot.memoryFlags;
        memoryJson = snapshot.memoryJson;
        contentVersion = snapshot.contentVersion;
        revision = snapshot.revision;
        sceneId = snapshot.sceneId;
        status = snapshot.status;
        stateJson = snapshot.stateJson;
        progressExists = snapshot.progressExists;
        eventCount = snapshot.eventCount;
        pauseOtherChapterCount = snapshot.pauseOtherChapterCount;
        transactionActive = false;
        ++rollbackCount;
        return true;
    }

    std::string_view driverName() const noexcept override { return "story-test"; }
    const std::string& lastError() const noexcept override { return error; }

    bool connected = false;
    bool transactionActive = false;
    bool failEventInsert = false;
    std::uint64_t gold = 10;
    std::uint32_t inventoryQuantity = 2;
    int affinity = 1;
    std::string memoryFlags = "met";
    std::string memoryJson = R"({"facts":["met"]})";
    int contentVersion = 3;
    std::uint64_t revision = 4;
    std::string sceneId = "start";
    std::string status = "active";
    std::string stateJson =
        R"({"flags":["saved"],"visited_scenes":["start"],"unlocked_chapters":["bonus"],"chapter_completed":false})";
    bool progressExists = true;
    int eventCount = 0;
    int beginCount = 0;
    int commitCount = 0;
    int rollbackCount = 0;
    int pauseOtherChapterCount = 0;

private:
    struct Snapshot {
        std::uint64_t gold = 0;
        std::uint32_t inventoryQuantity = 0;
        int affinity = 0;
        std::string memoryFlags;
        std::string memoryJson;
        int contentVersion = 0;
        std::uint64_t revision = 0;
        std::string sceneId;
        std::string status;
        std::string stateJson;
        bool progressExists = false;
        int eventCount = 0;
        int pauseOtherChapterCount = 0;
    } snapshot;

    bool fail(std::string message)
    {
        error = std::move(message);
        return false;
    }

    std::string error;
};

story::ChapterDefinition chapterDefinition()
{
    story::ChapterDefinition chapter;
    chapter.schemaVersion = 2;
    chapter.chapterId = QStringLiteral("persistence_test");
    chapter.contentVersion = 3;
    chapter.entrySceneId = QStringLiteral("start");
    chapter.title = {QStringLiteral("持久化测试"), QStringLiteral("Persistence Test")};
    story::SceneDefinition start;
    start.id = QStringLiteral("start");
    start.title = {QStringLiteral("起点"), QStringLiteral("Start")};
    start.narration = {QStringLiteral("开始。"), QStringLiteral("Begin.")};
    story::SceneDefinition end;
    end.id = QStringLiteral("end");
    end.title = {QStringLiteral("终点"), QStringLiteral("End")};
    end.narration = {QStringLiteral("结束。"), QStringLiteral("End.")};
    chapter.scenes = {start, end};
    return chapter;
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    bool passed = true;

    auto driver = std::make_unique<StoryDatabaseDriver>();
    auto* databaseState = driver.get();
    database::DBConfig config;
    config.driver = database::DatabaseDriverKind::Noop;
    auto connection = std::make_shared<database::DBConnection>(config, std::move(driver));
    passed &= expect(connection->connect(), "test database connects");

    const auto chapter = chapterDefinition();
    persistence::StoryPersistenceService service(connection);
    const auto loaded = service.load(chapter, 42);
    passed &= expect(loaded.success && loaded.found, "existing story state restores");
    passed &= expect(loaded.state.revision == 4 && loaded.state.gold == 10
                         && loaded.state.inventory.value(7) == 2
                         && loaded.state.flags.contains(QStringLiteral("saved"))
                         && loaded.state.flags.contains(QStringLiteral("legacy"))
                         && loaded.state.visitedScenes.contains(QStringLiteral("start"))
                         && loaded.state.unlockedChapters.contains(QStringLiteral("bonus")),
                     "chapter, gold, inventory, flags, visits, and unlocks restore");
    passed &= expect(loaded.state.npcAffinities.value(QStringLiteral("owl_keeper")) == 1
                         && loaded.state.npcMemoryFacts.value(QStringLiteral("owl_keeper"))
                                .contains(QStringLiteral("met"))
                         && loaded.state.completedChapters.contains(
                             QStringLiteral("earlier_chapter"))
                         && loaded.state.unlockedChapters.contains(
                             QStringLiteral("side_story"))
                         && loaded.state.characterAttributes.value(QStringLiteral("house"))
                                == QStringLiteral("Ravenclaw")
                         && loaded.state.characterAttributes.value(QStringLiteral("wand_wood"))
                                == QStringLiteral("oak"),
                     "NPC memory, completed chapters, and character attributes restore");

    story::StoryExecutionResult result;
    result.success = true;
    result.actionId = QStringLiteral("finish_purchase");
    result.delta.goldChange = -3;
    result.delta.inventoryChanges.push_back({7, -1});
    result.delta.affinityChanges.push_back({QStringLiteral("owl_keeper"), 2});
    result.delta.npcMemoryChanges.push_back(
        {QStringLiteral("owl_keeper"), QStringLiteral("helped"), true});
    result.delta.flagsToAdd.insert(QStringLiteral("done"));
    result.delta.nextSceneId = QStringLiteral("end");
    result.delta.completeChapter = true;

    QString error;
    passed &= expect(service.commit(chapter, loaded.state, result, error),
                     "multi-table story delta commits");
    passed &= expect(databaseState->gold == 7 && databaseState->inventoryQuantity == 1
                         && databaseState->affinity == 3
                         && databaseState->memoryJson.find("helped") != std::string::npos
                         && databaseState->revision == 5
                         && databaseState->sceneId == "end"
                         && databaseState->status == "completed"
                         && databaseState->eventCount == 1,
                     "gold, inventory, NPC memory, progress, and event log commit together");

    passed &= expect(!service.commit(chapter, loaded.state, result, error)
                         && error.contains(QStringLiteral("another session")),
                     "stale story revision is rejected");

    story::StoryRuntimeState current = loaded.state;
    story::StoryEngine engine;
    passed &= expect(engine.applyDelta(current, result.delta, &error),
                     "successful delta can prepare retry state");
    current.revision = 5;
    current.contentVersion = chapter.contentVersion;
    story::StoryExecutionResult second;
    second.success = true;
    second.actionId = QStringLiteral("postscript");
    second.delta.goldChange = -1;
    databaseState->failEventInsert = true;
    const auto goldBeforeFailure = databaseState->gold;
    const auto revisionBeforeFailure = databaseState->revision;
    passed &= expect(!service.commit(chapter, current, second, error),
                     "event log failure rejects the transaction");
    passed &= expect(databaseState->gold == goldBeforeFailure
                         && databaseState->revision == revisionBeforeFailure
                         && databaseState->eventCount == 1
                         && databaseState->rollbackCount >= 2,
                     "failed multi-table commit rolls every write back");

    databaseState->failEventInsert = false;
    databaseState->progressExists = false;
    story::StoryRuntimeState migratedCompleted;
    migratedCompleted.characterId = 42;
    migratedCompleted.chapterId = chapter.chapterId;
    migratedCompleted.contentVersion = chapter.contentVersion;
    migratedCompleted.currentSceneId = chapter.entrySceneId;
    migratedCompleted.gold = databaseState->gold;
    migratedCompleted.chapterCompleted = true;
    migratedCompleted.completedChapters.insert(chapter.chapterId);
    story::StoryExecutionResult migrationEntry;
    migrationEntry.success = true;
    migrationEntry.delta.scenesVisitedToAdd.insert(chapter.entrySceneId);
    passed &= expect(service.commit(chapter, migratedCompleted, migrationEntry, error)
                         && databaseState->pauseOtherChapterCount == 1
                         && databaseState->status == "completed",
                     "first generic save pauses the legacy active row even when already completed");

    return passed ? 0 : 1;
}
