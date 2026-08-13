#pragma once

#include <cstdint>
#include <string>

namespace arcane::application::dto {

struct LoginRequestDTO {
    std::string studentName;
    std::string house;
};

struct ChatSendRequestDTO {
    std::string channel;
    std::string text;
};

struct LocationMoveRequestDTO {
    std::string locationId;
};

struct CourseActionRequestDTO {
    std::string courseId;
};

struct ItemActionRequestDTO {
    std::string itemId;
    std::string npcId;
};

struct QuestProgressRequestDTO {
    std::uint64_t characterId = 0;
    std::uint64_t questId = 0;
    std::uint32_t progress = 0;
    std::string status;
};

// Request body for "use the Marauder's Map". roomId == 0 means "reveal the whole campus".
struct MapUseRequestDTO {
    std::uint64_t characterId = 0;
    std::uint64_t roomId = 0;
};

// Request body for listing a character's inventory (used by the refreshed InventoryService).
struct InventoryListRequestDTO {
    std::uint64_t characterId = 0;
};

} // namespace arcane::application::dto
