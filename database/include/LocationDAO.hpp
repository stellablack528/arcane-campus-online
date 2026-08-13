#pragma once

#include "DBConnection.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace arcane::database {

struct PlayerLocationRecord {
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

class LocationDAO final {
public:
    explicit LocationDAO(std::shared_ptr<DBConnection> connection) noexcept;

    // Upsert the player's current location; used when a character moves or updates position.
    [[nodiscard]] bool upsertLocation(std::uint64_t characterId,
                                      std::uint64_t roomId,
                                      double posX,
                                      double posY,
                                      bool isOnline);

    [[nodiscard]] bool setOnline(std::uint64_t characterId, bool isOnline);

    // Returns all online players with their location joined with character + room info.
    [[nodiscard]] std::vector<PlayerLocationRecord> getOnlinePlayers() const;

    // Returns online players located in a specific room.
    [[nodiscard]] std::vector<PlayerLocationRecord> getOnlinePlayersInRoom(std::uint64_t roomId) const;

    // Returns the location of a single player (may be offline).
    [[nodiscard]] std::optional<PlayerLocationRecord> getPlayerLocation(std::uint64_t characterId) const;

    // Removes the player's stored location entirely (e.g. on character logout).
    [[nodiscard]] bool deleteLocation(std::uint64_t characterId);

private:
    std::shared_ptr<DBConnection> connection_;
};

} // namespace arcane::database
