#pragma once

#include "application/dto/CampusDTO.hpp"
#include "application/vo/CampusVO.hpp"

#include <memory>

namespace arcane::database {
class InventoryDAO;
class LocationDAO;
} // namespace arcane::database

namespace arcane::application::service {

// Implements the "Marauder's Map" use-case: verify the character owns the map item,
// then reveal online players (optionally filtered by room).
class MapService final {
public:
    explicit MapService(std::shared_ptr<database::InventoryDAO> inventoryDao,
                        std::shared_ptr<database::LocationDAO> locationDao) noexcept;

    [[nodiscard]] vo::MapResultVO useMaraudersMap(const dto::MapUseRequestDTO& request) const;

private:
    [[nodiscard]] bool ownsMaraudersMap(std::uint64_t characterId) const;
    [[nodiscard]] static std::string pickSpellQuote();

    std::shared_ptr<database::InventoryDAO> inventoryDao_;
    std::shared_ptr<database::LocationDAO> locationDao_;
};

} // namespace arcane::application::service
