#include "application/service/QuestService.hpp"

#include "QuestDAO.hpp"

#include <unordered_map>
#include <utility>

namespace arcane::application::service {

QuestService::QuestService(std::shared_ptr<database::QuestDAO> questDao) noexcept
    : questDao_(std::move(questDao))
{
}

vo::QuestListVO QuestService::getCharacterQuests(std::uint64_t characterId) const
{
    if (!questDao_) {
        return {};
    }

    std::unordered_map<std::uint64_t, database::QuestRecord> definitions;
    for (const auto& quest : questDao_->getAllQuests()) {
        definitions.emplace(quest.questId, quest);
    }

    vo::QuestListVO quests;
    for (const auto& progress : questDao_->getCharacterQuests(characterId)) {
        const auto definition = definitions.find(progress.questId);
        if (definition == definitions.end()) {
            continue;
        }
        quests.push_back(vo::QuestVO{definition->second.questId, definition->second.questName,
                                     definition->second.description, progress.progress, progress.status,
                                     definition->second.rewardGold, definition->second.rewardExperience});
    }
    return quests;
}

vo::OperationResultVO QuestService::updateProgress(const dto::QuestProgressRequestDTO& request) const
{
    if (!questDao_) {
        return {false, "Quest storage is not configured."};
    }
    if (request.status != "not_started" && request.status != "in_progress"
        && request.status != "completed" && request.status != "claimed") {
        return {false, "Invalid quest status."};
    }
    const database::CharacterQuestRecord record{request.characterId, request.questId,
                                                 request.progress, request.status};
    return questDao_->updateCharacterQuest(record)
        ? vo::OperationResultVO{true, "Quest progress updated."}
        : vo::OperationResultVO{false, "Quest progress could not be updated."};
}

} // namespace arcane::application::service
