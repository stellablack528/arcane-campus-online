#include "application/service/StoryProgressService.hpp"

#include "CharacterDAO.hpp"

#include <utility>

namespace arcane::application::service {

StoryProgressService::StoryProgressService(std::shared_ptr<database::StoryDAO> storyDao,
                                           std::shared_ptr<database::CharacterDAO> characterDao) noexcept
    : storyDao_(std::move(storyDao))
    , characterDao_(std::move(characterDao))
{
}

bool StoryProgressService::saveState(const database::StoryStateRecord& state) const
{
    return storyDao_ && storyDao_->upsertStoryState(state);
}

std::optional<database::StoryStateRecord> StoryProgressService::loadState(std::uint64_t characterId) const
{
    return storyDao_ ? storyDao_->getStoryState(characterId) : std::nullopt;
}

bool StoryProgressService::saveWand(const database::CharacterWandRecord& wand) const
{
    return storyDao_ && storyDao_->upsertWand(wand);
}

std::optional<database::CharacterWandRecord> StoryProgressService::loadWand(std::uint64_t characterId) const
{
    return storyDao_ ? storyDao_->getWand(characterId) : std::nullopt;
}

bool StoryProgressService::markWandClaimed(std::uint64_t characterId) const
{
    return storyDao_ && storyDao_->markWandClaimed(characterId);
}

bool StoryProgressService::saveNpcMemory(const database::NpcMemoryRecord& memory) const
{
    return storyDao_ && storyDao_->upsertNpcMemory(memory);
}

std::vector<database::NpcMemoryRecord> StoryProgressService::loadNpcMemories(std::uint64_t characterId) const
{
    return storyDao_ ? storyDao_->getNpcMemories(characterId)
                     : std::vector<database::NpcMemoryRecord>{};
}

GoldResult StoryProgressService::spendGold(std::uint64_t characterId, std::uint64_t amount) const
{
    if (!characterDao_) return {false, "Character storage is unavailable.", 0};
    const auto character = characterDao_->getCharacterById(characterId);
    if (!character) return {false, "The character record could not be loaded.", 0};
    if (character->gold < amount) {
        return {false, "You do not have enough Galleons for that purchase.", character->gold};
    }
    const auto remaining = character->gold - amount;
    if (!characterDao_->updateCharacterGold(characterId, remaining)) {
        return {false, "The shop could not complete the payment.", character->gold};
    }
    return {true, "Payment completed.", remaining};
}

bool StoryProgressService::refundGold(std::uint64_t characterId, std::uint64_t amount) const
{
    if (!characterDao_) return false;
    const auto character = characterDao_->getCharacterById(characterId);
    return character && characterDao_->updateCharacterGold(characterId, character->gold + amount);
}

std::optional<std::uint64_t> StoryProgressService::currentGold(std::uint64_t characterId) const
{
    if (!characterDao_) return std::nullopt;
    const auto character = characterDao_->getCharacterById(characterId);
    return character ? std::optional<std::uint64_t>{character->gold} : std::nullopt;
}

} // namespace arcane::application::service
