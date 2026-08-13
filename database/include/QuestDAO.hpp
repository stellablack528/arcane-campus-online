#pragma once

#include "DBConnection.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace arcane::database {

struct QuestRecord {
    std::uint64_t questId = 0;
    std::string questName;
    std::string description;
    std::uint32_t rewardGold = 0;
    std::uint32_t rewardExperience = 0;
};

struct CharacterQuestRecord {
    std::uint64_t characterId = 0;
    std::uint64_t questId = 0;
    std::uint32_t progress = 0;
    std::string status;
};

class QuestDAO final {
public:
    explicit QuestDAO(std::shared_ptr<DBConnection> connection) noexcept;

    [[nodiscard]] bool createQuest(const QuestRecord& quest);
    [[nodiscard]] std::optional<QuestRecord> getQuestById(std::uint64_t questId) const;
    [[nodiscard]] std::vector<QuestRecord> getAllQuests() const;
    [[nodiscard]] bool updateQuest(const QuestRecord& quest);
    [[nodiscard]] bool deleteQuest(std::uint64_t questId);

    [[nodiscard]] bool assignQuest(const CharacterQuestRecord& characterQuest);
    [[nodiscard]] std::vector<CharacterQuestRecord> getCharacterQuests(std::uint64_t characterId) const;
    [[nodiscard]] bool updateCharacterQuest(const CharacterQuestRecord& characterQuest);
    [[nodiscard]] bool removeCharacterQuest(std::uint64_t characterId, std::uint64_t questId);

private:
    std::shared_ptr<DBConnection> connection_;
};

} // namespace arcane::database
