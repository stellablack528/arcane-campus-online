#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>

namespace arcane::application::open_world {

// A provider-neutral, read-only view of the authoritative Hogwarts world state.
// It deliberately contains no Qt, database, network, or TaleState types.
struct OpenWorldSnapshot {
    std::uint64_t characterId = 0;
    std::string chapterId;
    int contentVersion = 0;
    std::uint64_t acceptedStoryTurns = 0;
    std::string currentSceneId;
    std::string house;
    std::uint64_t gold = 0;

    std::set<std::string> flags;
    std::map<std::uint64_t, std::uint32_t> inventory;
    std::map<std::string, int> npcRelationships;
    std::map<std::string, std::set<std::string>> npcMemoryFacts;
    std::map<std::string, std::string> characterAttributes;
    std::set<std::string> visitedScenes;
    std::set<std::string> completedChapters;
    std::set<std::string> unlockedChapters;

    bool chapterCompleted = false;
    bool mainStoryComplete = false;
    bool openWorldHandoffReady = false;

    [[nodiscard]] bool readyForOpenWorld() const noexcept
    {
        return mainStoryComplete && openWorldHandoffReady;
    }

    bool operator==(const OpenWorldSnapshot&) const = default;
};

} // namespace arcane::application::open_world
