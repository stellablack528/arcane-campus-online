#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace arcane::application::vo {

struct OperationResultVO {
    bool success = false;
    std::string message;
};

struct LoginVO {
    bool success = false;
    std::string message;
    std::string studentName;
    std::string house;
    std::string location;
};

struct CampusMessageVO {
    std::string channel;
    std::string speaker;
    std::string text;
};

struct PlayerStatusVO {
    std::string location;
    std::string state;
};

struct QuestVO {
    std::uint64_t questId = 0;
    std::string name;
    std::string description;
    std::uint32_t progress = 0;
    std::string status;
    std::uint32_t rewardGold = 0;
    std::uint32_t rewardExperience = 0;
};

using QuestListVO = std::vector<QuestVO>;

// A single online player revealed by the Marauder's Map.
struct MapPlayerVO {
    std::uint64_t characterId = 0;
    std::string nickname;
    std::string house;
    std::string roomName;
    double posX = 0.0;
    double posY = 0.0;
    std::string statusEmoji;
};

// Full result of using the Marauder's Map.
struct MapResultVO {
    bool success = false;
    std::string message;
    std::string spellQuote;
    std::uint32_t onlineCount = 0;
    std::vector<MapPlayerVO> players;
};

// A single inventory item shown to the player.
struct InventoryItemVO {
    std::uint64_t itemId = 0;
    std::string itemName;
    std::string itemType;
    std::string description;
    std::uint32_t quantity = 0;
    bool stackable = false;
};

// Full inventory listing response.
struct InventoryListVO {
    bool success = false;
    std::string message;
    std::uint64_t characterId = 0;
    std::uint32_t totalItems = 0;
    std::vector<InventoryItemVO> items;
};

// 学院积分变化事件（用于 HouseRankingWidget 显示"攒了多少分/丢了多少分"）。
struct HousePointsChangeVO {
    std::string house;         // 学院名（Gryffindor / Slytherin / Hufflepuff / Ravenclaw）
    std::int32_t delta = 0;    // 分数变化（正=加分，负=扣分）
    std::string reason;        // 变化原因（如 "Curfew violation at Forbidden Forest"）
};

} // namespace arcane::application::vo
