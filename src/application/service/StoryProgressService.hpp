#pragma once

#include "StoryDAO.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace arcane::database {
class CharacterDAO;
}

namespace arcane::application::service {

struct GoldResult {
    bool success = false;
    std::string message;
    std::uint64_t remainingGold = 0;
};

class StoryProgressService final {
public:
    StoryProgressService(std::shared_ptr<database::StoryDAO> storyDao,
                         std::shared_ptr<database::CharacterDAO> characterDao) noexcept;

    [[nodiscard]] bool saveState(const database::StoryStateRecord& state) const;
    [[nodiscard]] std::optional<database::StoryStateRecord> loadState(std::uint64_t characterId) const;
    [[nodiscard]] bool saveWand(const database::CharacterWandRecord& wand) const;
    [[nodiscard]] std::optional<database::CharacterWandRecord> loadWand(std::uint64_t characterId) const;
    [[nodiscard]] bool markWandClaimed(std::uint64_t characterId) const;
    [[nodiscard]] bool saveNpcMemory(const database::NpcMemoryRecord& memory) const;
    [[nodiscard]] std::vector<database::NpcMemoryRecord> loadNpcMemories(std::uint64_t characterId) const;

    [[nodiscard]] GoldResult spendGold(std::uint64_t characterId, std::uint64_t amount) const;
    [[nodiscard]] bool refundGold(std::uint64_t characterId, std::uint64_t amount) const;
    [[nodiscard]] std::optional<std::uint64_t> currentGold(std::uint64_t characterId) const;

private:
    std::shared_ptr<database::StoryDAO> storyDao_;
    std::shared_ptr<database::CharacterDAO> characterDao_;
};

} // namespace arcane::application::service
