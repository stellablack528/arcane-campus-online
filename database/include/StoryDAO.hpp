#pragma once

#include "DBConnection.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace arcane::database {

struct StoryStateRecord {
    std::uint64_t characterId = 0;
    std::string chapterId;
    std::string sceneId;
    std::string stage;
    std::string flags;
};

struct CharacterWandRecord {
    std::uint64_t characterId = 0;
    std::string woodId;
    std::string coreId;
    std::string woodName;
    std::string coreName;
    double lengthInches = 11.0;
    std::string flexibility;
    std::string handleStyle;
    std::string finish;
    std::string colorTone;
    std::string engraving;
    bool claimed = false;
};

struct NpcMemoryRecord {
    std::uint64_t characterId = 0;
    std::string npcId;
    int affinity = 0;
    std::string memoryFlags;
};

class StoryDAO final {
public:
    explicit StoryDAO(std::shared_ptr<DBConnection> connection) noexcept;

    [[nodiscard]] bool upsertStoryState(const StoryStateRecord& state);
    [[nodiscard]] std::optional<StoryStateRecord> getStoryState(std::uint64_t characterId) const;

    [[nodiscard]] bool upsertWand(const CharacterWandRecord& wand);
    [[nodiscard]] std::optional<CharacterWandRecord> getWand(std::uint64_t characterId) const;
    [[nodiscard]] bool markWandClaimed(std::uint64_t characterId);

    [[nodiscard]] bool upsertNpcMemory(const NpcMemoryRecord& memory);
    [[nodiscard]] std::vector<NpcMemoryRecord> getNpcMemories(std::uint64_t characterId) const;

private:
    std::shared_ptr<DBConnection> connection_;
};

} // namespace arcane::database
