#include "application/world/FirstTermStore.hpp"

#include "DBConnection.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <string>
#include <utility>

namespace arcane::application::world {

FirstTermStore::FirstTermStore(std::shared_ptr<database::DBConnection> connection)
    : connection_(std::move(connection))
{
    if (connection_ && connection_->isConnected())
        ready_ = connection_->execute(
            "CREATE TABLE IF NOT EXISTS character_term_states ("
            "character_id BIGINT PRIMARY KEY, state_json TEXT NOT NULL)");
}

std::optional<FirstTermState> FirstTermStore::load(std::uint64_t characterId) const
{
    if (!ready_) return std::nullopt;
    const auto result = connection_->query(
        "SELECT state_json FROM character_term_states WHERE character_id = ?",
        {std::to_string(characterId)});
    if (!result || result->rows.empty()) return std::nullopt;
    const auto bytes = QByteArray::fromStdString(result->rows.front().at("state_json"));
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) return std::nullopt;
    const auto object = document.object();
    if (object.value("version").toInt() != 1) return std::nullopt;
    FirstTermState state;
    state.day = object.value("day").toInt(1);
    state.phase = object.value("phase").toInt(0);
    state.health = object.value("health").toInt(90);
    state.reputation = object.value("reputation").toInt(0);
    state.attended = object.value("attended").toInt(0);
    state.missed = object.value("missed").toInt(0);
    state.meals = object.value("meals").toInt(0);
    state.forestChoice = object.value("forest_choice").toBool();
    state.critical = object.value("critical").toBool();
    state.complete = object.value("complete").toBool();
    state.eraId = object.value("era_id").toString(QStringLiteral("harry")).toStdString();
    state.accidentPending = object.value("accident_pending").toBool();
    state.accidentOutcome = object.value("accident_outcome").toString().toStdString();
    state.clueFound = object.value("clue_found").toBool();
    state.followupSeen = object.value("followup_seen").toBool();
    state.clueShared = object.value("clue_shared").toBool();
    const auto subjects = object.value("subjects").toArray();
    if (subjects.size() != 4 || state.day < 1 || state.day > 36 || state.phase < 0
        || state.phase > 6 || state.health < 0 || state.health > 100
        || state.reputation < -100 || state.reputation > 100
        || (state.eraId != "parents" && state.eraId != "harry" && state.eraId != "riddle")) return std::nullopt;
    for (int i = 0; i < 4; ++i) state.subjects[i] = subjects.at(i).toInt();
    return state;
}

bool FirstTermStore::save(std::uint64_t characterId, const FirstTermState& state) const
{
    if (!ready_ || !characterId) return false;
    QJsonArray subjects;
    for (int value : state.subjects) subjects.append(value);
    QJsonObject object{{"version", 1}, {"day", state.day}, {"phase", state.phase},
                       {"health", state.health}, {"reputation", state.reputation},
                       {"attended", state.attended}, {"missed", state.missed},
                       {"meals", state.meals}, {"forest_choice", state.forestChoice},
                       {"critical", state.critical}, {"complete", state.complete},
                       {"era_id", QString::fromStdString(state.eraId)},
                       {"accident_pending", state.accidentPending},
                       {"accident_outcome", QString::fromStdString(state.accidentOutcome)},
                       {"clue_found", state.clueFound}, {"followup_seen", state.followupSeen},
                       {"clue_shared", state.clueShared},
                       {"subjects", subjects}};
    const auto data = QJsonDocument(object).toJson(QJsonDocument::Compact).toStdString();
    const auto id = std::to_string(characterId);
    const auto existing = connection_->query(
        "SELECT character_id FROM character_term_states WHERE character_id = ?", {id});
    if (!existing) return false;
    if (existing->rows.empty())
        return connection_->execute(
            "INSERT INTO character_term_states (character_id, state_json) VALUES (?, ?)", {id, data});
    return connection_->execute(
        "UPDATE character_term_states SET state_json = ? WHERE character_id = ?", {data, id});
}

} // namespace arcane::application::world
