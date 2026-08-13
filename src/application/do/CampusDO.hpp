#pragma once

#include <cstdint>
#include <string>

namespace arcane::application::do_model {

struct PlayerSessionDO {
    std::uint64_t userId = 0;
    std::uint64_t characterId = 0;
    std::string nickname;
    std::string house;
    std::string currentLocation = "Great Hall";
    std::string currentState = "Breakfast";
    bool active = false;
};

struct ChatMessageDO {
    std::string channel;
    std::string speaker;
    std::string content;
    std::string roomId;
};

struct QuestProgressDO {
    std::uint64_t characterId = 0;
    std::uint64_t questId = 0;
    std::uint32_t progress = 0;
    std::string status;
};

// Mirrors a row joined from player_locations + characters + rooms.
struct PlayerLocationDO {
    std::uint64_t characterId = 0;
    std::string nickname;
    std::string house;
    std::uint64_t roomId = 0;
    std::string roomName;
    double posX = 0.0;
    double posY = 0.0;
    bool isOnline = false;
    std::string lastUpdate;
};

} // namespace arcane::application::do_model
