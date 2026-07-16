#pragma once

#include "../database/include/DBConnection.hpp"
#include "DO/PlayerLocationDO.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace arcane::database {

class LocationDAO final {
public:
    explicit LocationDAO(std::shared_ptr<DBConnection> connection) noexcept;

    [[nodiscard]] std::optional<PlayerLocationDO> findByCharacterId(std::uint64_t characterId) const;
    [[nodiscard]] std::vector<PlayerLocationDO> findAllOnline() const;
    [[nodiscard]] std::vector<PlayerLocationDO> findOnlineByRoom(const std::string& roomName) const;
    [[nodiscard]] bool updatePosition(std::uint64_t characterId,
                                      const std::string& roomName,
                                      std::int32_t xPos, std::int32_t yPos);

private:
    std::shared_ptr<DBConnection> connection_;
    [[nodiscard]] static PlayerLocationDO parseRow(const QueryRow& row);
};

} // namespace arcane::database
