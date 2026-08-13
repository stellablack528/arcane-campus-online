#include "QuestDAO.hpp"

#include "RecordMapper.hpp"

#include <utility>

namespace arcane::database {

QuestDAO::QuestDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

bool QuestDAO::createQuest(const QuestRecord& quest)
{
    return connection_->execute(
        "INSERT INTO quests (quest_name, description, reward_gold, reward_exp) VALUES (?, ?, ?, ?)",
        {quest.questName, quest.description, std::to_string(quest.rewardGold),
         std::to_string(quest.rewardExperience)});
}

std::optional<QuestRecord> QuestDAO::getQuestById(std::uint64_t questId) const
{
    const auto result = connection_->query("SELECT * FROM quests WHERE quest_id = ?", {std::to_string(questId)});
    if (!result || result->rows.empty()) {
        return std::nullopt;
    }
    const auto& row = result->rows.front();
    return QuestRecord{detail::integer<std::uint64_t>(row, "quest_id"), detail::value(row, "quest_name"),
                       detail::value(row, "description"), detail::integer<std::uint32_t>(row, "reward_gold"),
                       detail::integer<std::uint32_t>(row, "reward_exp")};
}

std::vector<QuestRecord> QuestDAO::getAllQuests() const
{
    const auto result = connection_->query("SELECT * FROM quests ORDER BY quest_id ASC");
    std::vector<QuestRecord> quests;
    if (!result) {
        return quests;
    }
    quests.reserve(result->rows.size());
    for (const auto& row : result->rows) {
        quests.push_back(QuestRecord{detail::integer<std::uint64_t>(row, "quest_id"),
                                     detail::value(row, "quest_name"), detail::value(row, "description"),
                                     detail::integer<std::uint32_t>(row, "reward_gold"),
                                     detail::integer<std::uint32_t>(row, "reward_exp")});
    }
    return quests;
}

bool QuestDAO::updateQuest(const QuestRecord& quest)
{
    return connection_->execute(
        "UPDATE quests SET quest_name = ?, description = ?, reward_gold = ?, reward_exp = ? WHERE quest_id = ?",
        {quest.questName, quest.description, std::to_string(quest.rewardGold),
         std::to_string(quest.rewardExperience), std::to_string(quest.questId)});
}

bool QuestDAO::deleteQuest(std::uint64_t questId)
{
    return connection_->execute("DELETE FROM quests WHERE quest_id = ?", {std::to_string(questId)});
}

bool QuestDAO::assignQuest(const CharacterQuestRecord& characterQuest)
{
    return connection_->execute(
        "INSERT INTO character_quests (character_id, quest_id, progress, status) VALUES (?, ?, ?, ?)",
        {std::to_string(characterQuest.characterId), std::to_string(characterQuest.questId),
         std::to_string(characterQuest.progress), characterQuest.status});
}

std::vector<CharacterQuestRecord> QuestDAO::getCharacterQuests(std::uint64_t characterId) const
{
    const auto result = connection_->query("SELECT * FROM character_quests WHERE character_id = ? ORDER BY quest_id ASC",
                                           {std::to_string(characterId)});
    std::vector<CharacterQuestRecord> quests;
    if (!result) {
        return quests;
    }
    quests.reserve(result->rows.size());
    for (const auto& row : result->rows) {
        quests.push_back(CharacterQuestRecord{detail::integer<std::uint64_t>(row, "character_id"),
                                              detail::integer<std::uint64_t>(row, "quest_id"),
                                              detail::integer<std::uint32_t>(row, "progress"),
                                              detail::value(row, "status")});
    }
    return quests;
}

bool QuestDAO::updateCharacterQuest(const CharacterQuestRecord& characterQuest)
{
    return connection_->execute(
        "UPDATE character_quests SET progress = ?, status = ? WHERE character_id = ? AND quest_id = ?",
        {std::to_string(characterQuest.progress), characterQuest.status,
         std::to_string(characterQuest.characterId), std::to_string(characterQuest.questId)});
}

bool QuestDAO::removeCharacterQuest(std::uint64_t characterId, std::uint64_t questId)
{
    return connection_->execute("DELETE FROM character_quests WHERE character_id = ? AND quest_id = ?",
                                {std::to_string(characterId), std::to_string(questId)});
}

} // namespace arcane::database
