#pragma once

#include "../database/include/DBConnection.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace arcane::database {

struct RoomRecord {
    std::uint64_t roomId = 0;
    std::string roomName;
    std::string roomType;
    std::string description;
    std::uint32_t maxPlayers = 0;
};

class RoomDAO final {
public:
    explicit RoomDAO(std::shared_ptr<DBConnection> connection) noexcept;
    [[nodiscard]] bool createRoom(const RoomRecord& room);
    [[nodiscard]] std::optional<RoomRecord> getRoomById(std::uint64_t roomId) const;
    [[nodiscard]] std::vector<RoomRecord> getAllRooms() const;
    [[nodiscard]] bool updateRoom(const RoomRecord& room);
    [[nodiscard]] bool deleteRoom(std::uint64_t roomId);
private:
    std::shared_ptr<DBConnection> connection_;
};

} // namespace arcane::database
