#include "application/open_world/npc/NpcInteraction.hpp"
#include "application/open_world/npc/CustomNpcStore.hpp"
#include "DBConnection.hpp"
#include "LocalSaveBootstrap.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QTemporaryDir>
#include <iostream>

namespace npc = arcane::application::open_world::npc;
namespace db = arcane::database;
using namespace Qt::StringLiterals;

static bool expect(bool pass, const char* name)
{
    std::cout << (pass ? "PASS " : "FAIL ") << name << '\n';
    return pass;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    npc::Catalog catalog;
    QString error;
    const auto errors = catalog.loadBuiltIns();
    if (!expect(errors.isEmpty(), "discover and validate built-in resource packs")) {
        std::cerr << errors.join(u'\n').toStdString() << '\n';
        return 1;
    }
    QFile file(u":/story/worlds/npcs/riddle.json"_s);
    if (!file.open(QIODevice::ReadOnly)) return 1;
    const auto riddlePack = file.readAll();
    const auto* tom = catalog.find(u"riddle.tom_riddle"_s);
    const auto* hermione = catalog.find(u"harry.hermione_granger"_s);
    if (!expect(tom && hermione && tom->persona.sources.size() == 2
                && tom->persona.instructions(true) != hermione->persona.instructions(true),
                "canonical profiles expose distinct, sourced instruction cards")) return 1;
    QSet<QString> eraIds;
    for (const auto& profile : catalog.profiles()) {
        eraIds.insert(profile.era);
        if (!expect(!profile.persona.sources.isEmpty() && !profile.persona.instructions(true).isEmpty()
                    && !profile.persona.instructions(false).isEmpty(), "each canonical card has bilingual instructions and sources")) return 1;
    }
    const auto* teenSnape = catalog.find(u"parents.severus_snape"_s);
    const auto* professorSnape = catalog.find(u"harry.severus_snape"_s);
    const auto parentsSnape = catalog.matchInput(u"找斯内普"_s, u"parents"_s);
    const auto harrySnape = catalog.matchInput(u"找斯内普"_s, u"harry"_s);
    if (!expect(eraIds.size() == 3 && catalog.profiles().size() >= 20 && teenSnape && professorSnape
                && teenSnape->persona.instructions(true) != professorSnape->persona.instructions(true)
                && parentsSnape && parentsSnape->npcId == teenSnape->id
                && harrySnape && harrySnape->npcId == professorSnape->id,
                "all three eras load and role variants resolve in the current era")) return 1;

    npc::Catalog original;
    auto isolated = QJsonDocument::fromJson(riddlePack).object();
    isolated.insert(u"era"_s, u"custom"_s);
    auto singleProfile = isolated.value(u"profiles"_s).toArray().first().toObject();
    singleProfile.insert(u"id"_s, u"custom.custom_new_student"_s);
    isolated.insert(u"profiles"_s, QJsonArray{singleProfile});
    auto customPack = QJsonDocument(isolated).toJson(QJsonDocument::Compact);
    customPack.replace("intelligent, ambitious, and guarded", "curious, generous, and patient");
    if (!expect(original.loadCustomPack(customPack, u"custom"_s, error)
                && original.find(u"custom.custom_new_student"_s)
                && original.find(u"custom.custom_new_student"_s)->persona.instructions(false).contains(u"generous"_s),
                "the same loader accepts author-supplied personality instructions")) return 1;
    auto missingInstructions = riddlePack;
    missingInstructions.replace("\"speech\"", "\"wrong_field\"");
    if (!expect(!original.loadPack(missingInstructions, u"riddle"_s, error)
                && !original.find(u"riddle.tom_riddle"_s), "malformed profile does not register")) return 1;

    const auto notes = catalog.matchInput(u"我想和汤姆聊聊咒语笔记。"_s, u"riddle"_s);
    const auto english = catalog.matchInput(u"Talk to Tom Riddle about spell notes"_s, u"riddle"_s);
    const auto explicitRequest = catalog.matchInput(u"/npc 汤姆：笔记"_s, u"riddle"_s);
    if (!expect(notes && english && explicitRequest && notes->npcId == tom->id && notes->topic == u"咒语笔记"_s
                && english->npcId == tom->id && english->topic == u"spell notes"_s
                && explicitRequest->topic == u"笔记"_s
                && !catalog.matchInput(u"大家知道汤姆吗？"_s, u"riddle"_s)
                && !catalog.matchInput(u"和汤姆森聊笔记"_s, u"riddle"_s),
                "explicit and natural inputs route locally; ordinary chat does not")) return 1;

    QTemporaryDir folder(QDir::current().filePath(u"npc-test-XXXXXX"_s));
    if (!folder.isValid()) return 1;
    {
        QFile example(u":/story/examples/custom_npcs_riddle.json"_s);
        if (!expect(example.open(QIODevice::ReadOnly), "open the documented author example")) return 1;
        const auto pack = example.readAll();
        npc::CustomNpcStore store(folder.path());
        npc::Catalog active;
        if (!expect(store.importPack(101, u"riddle"_s, pack, catalog, active, error), "import a local character pack")) {
            std::cerr << error.toStdString() << '\n'; return 1;
        }
        const auto* ada = active.find(u"riddle.custom_ada_reed"_s);
        if (!expect(ada && ada->customFields.value(u"favorite_number"_s).toInt() == 7
                    && ada->customFields.value(u"prefers_quiet"_s).toBool()
                    && ada->customFields.value(u"hobby"_s).toObject().value(u"zh"_s).toString() == u"修旧怀表"_s
                    && active.find(tom->id) && !catalog.find(ada->id),
                    "typed fields load without changing canonical content")) return 1;
        auto invalid = pack;
        invalid.replace("\"favorite_number\": 7", "\"favorite_number\": [7]");
        auto override = pack;
        override.replace("riddle.custom_ada_reed", "riddle.tom_riddle");
        auto duplicateAlias = pack;
        duplicateAlias.replace("Ada Reed\"]", "Tom\"]");
        auto unknownScene = pack;
        unknownScene.replace("\"library\"", "\"not_a_room\"");
        if (!expect(!store.importPack(101, u"riddle"_s, invalid, catalog, active, error)
                    && !store.importPack(101, u"riddle"_s, override, catalog, active, error)
                    && !store.importPack(101, u"riddle"_s, duplicateAlias, catalog, active, error)
                    && !store.importPack(101, u"riddle"_s, unknownScene, catalog, active, error)
                    && !store.importPack(101, u"harry"_s, pack, catalog, active, error),
                    "bad fields, canonical collisions, unknown scenes and wrong-era packs are rejected")) return 1;
        npc::Catalog restored;
        if (!expect(store.restore(101, u"riddle"_s, catalog, restored, error)
                    && restored.find(u"riddle.custom_ada_reed"_s) && active.find(u"riddle.custom_ada_reed"_s),
                    "failed imports leave disk and live catalog intact")) return 1;
        if (!expect(store.restore(102, u"riddle"_s, catalog, restored, error)
                    && !restored.find(u"riddle.custom_ada_reed"_s), "another player cannot see this local custom pack")) return 1;
        const auto request = active.matchInput(u"/npc 里德：怀表"_s, u"riddle"_s);
        std::string saveError;
        const auto save = db::LocalSaveBootstrap::open(
            std::filesystem::path(folder.filePath(u"custom.sqlite"_s).toStdWString()), &saveError);
        if (!expect(request && save && npc::Interaction(save).talk(active,
                    {101, u"riddle"_s, u"library"_s, 5, true}, *request).spoken,
                    "imported NPC uses the normal dialogue and memory flow")) return 1;
        auto renamed = pack;
        renamed.replace("Ada Reed", "Ada Reed Updated");
        if (!expect(store.importPack(101, u"riddle"_s, renamed, catalog, active, error)
                    && active.find(u"riddle.custom_ada_reed"_s)->name.en == u"Ada Reed Updated"_s,
                    "reimport updates an existing stable id")) return 1;
        if (!expect(store.restore(101, u"riddle"_s, catalog, restored, error)
                    && npc::Interaction(save).talk(restored, {101, u"riddle"_s, u"library"_s, 5, true},
                       {request->npcId, {}}).text == restored.find(request->npcId)->familiarGreeting.zh,
                    "renamed stable id retains memory after refreshing the catalog")) return 1;
    }
    const auto path = std::filesystem::path(folder.filePath(u"campus.sqlite"_s).toStdWString());
    const npc::TalkRequest greet{tom->id, {}};
    const npc::TalkContext lunch{101, u"riddle"_s, u"great_hall"_s, 2, true};
    const npc::TalkContext library{101, u"riddle"_s, u"library"_s, 5, true};
    std::string databaseError;
    {
        const auto connection = db::LocalSaveBootstrap::open(path, &databaseError);
        if (!connection) { std::cerr << databaseError << '\n'; return 1; }
        npc::Interaction interaction(connection);
        auto wrong = lunch;
        wrong.era = u"parents"_s;
        const auto wrongEra = interaction.talk(catalog, wrong, greet);
        wrong = library;
        wrong.phase = 1;
        const auto wrongTime = interaction.talk(catalog, wrong, greet);
        wrong = lunch;
        wrong.scene = u"courtyard"_s;
        const auto wrongScene = interaction.talk(catalog, wrong, greet);
        if (!expect(!wrongEra.spoken && wrongEra.text.contains(u"不在这个时代"_s) && !wrongTime.spoken
                    && !wrongScene.spoken && wrongScene.text.contains(u"大礼堂"_s),
                    "era, scene and time restrictions return useful feedback")) return 1;

        auto result = interaction.talk(catalog, lunch, greet);
        if (!expect(result.spoken && result.text == tom->firstMeeting.zh, "first meeting commits before dialogue")) return 1;
        result = interaction.talk(catalog, library, *notes);
        if (!expect(result.spoken && result.text == tom->topics.front().reply.zh, "a topic is recognized after moving scenes")) return 1;
        if (!expect(interaction.talk(catalog, library, greet).text == tom->familiarGreeting.zh
                    && tom->topics.front().repeatReply
                    && interaction.talk(catalog, library, *notes).text == tom->topics.front().repeatReply->zh,
                    "repeated interactions remember the earlier discussion")) return 1;

        const auto stored = connection->query("SELECT affinity, revision, memory_flags FROM npc_memories WHERE character_id = ? AND npc_id = ?",
                                               {"101", "riddle.tom_riddle"});
        if (!expect(stored && stored->rows.size() == 1 && stored->rows.front().at("affinity") == "0"
                    && stored->rows.front().at("revision") == "2"
                    && stored->rows.front().at("memory_flags") == "met;discussed_spell_notes",
                    "facts are saved once and do not farm affinity or revisions")) return 1;

        auto secondPlayer = lunch;
        secondPlayer.characterId = 102;
        if (!expect(interaction.talk(catalog, secondPlayer, greet).text == tom->firstMeeting.zh,
                    "another character does not inherit the first one's memories")) return 1;
        const auto fallback = interaction.talk(catalog, library, {tom->id, u"unknown"_s});
        if (!expect(fallback.spoken && fallback.text == tom->unknownTopic.zh, "unknown topics get in-character guidance")) return 1;
        if (!expect(connection->execute(
                "CREATE TRIGGER reject_npc_write BEFORE INSERT ON npc_memories BEGIN SELECT RAISE(ABORT, 'test'); END"),
                "install a save failure in disposable test database")) return 1;
        secondPlayer.characterId = 103;
        if (!expect(!interaction.talk(catalog, secondPlayer, greet).spoken,
                    "a storage failure does not publish a successful meeting")) return 1;
    }
    {
        const auto reopened = db::LocalSaveBootstrap::open(path, &databaseError);
        if (!reopened) return 1;
        const npc::Interaction restored(reopened);
        const auto result = restored.talk(catalog, library, greet);
        const auto facts = reopened->query("SELECT memory_flags FROM npc_memories WHERE character_id = ? AND npc_id = ?",
                                           {"101", "riddle.tom_riddle"});
        if (!expect(result.spoken && result.text == tom->familiarGreeting.zh && facts && facts->rows.size() == 1
                    && facts->rows.front().at("memory_flags") == "met;discussed_spell_notes"
                    && restored.talk(catalog, library, *notes).text == tom->topics.front().repeatReply->zh,
                    "the NPC remembers the meeting after reopening SQLite")) return 1;
    }
    return 0;
}
