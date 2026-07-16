#pragma once

#include "../database/include/DBConnection.hpp"
#include "DO/PlayerInventoryDO.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace arcane::database {

struct ItemRecord {
    std::uint64_t itemId = 0;
    std::string itemName;
    std::string itemType;
    std::string description;
    bool stackable = false;
};

struct InventoryRecord {
    std::uint64_t inventoryId = 0;
    std::uint64_t characterId = 0;
    std::uint64_t itemId = 0;
    std::uint32_t quantity = 0;
};

class InventoryDAO final {
public:
    explicit InventoryDAO(std::shared_ptr<DBConnection> connection) noexcept;

    // ---- 基础 CRUD ----
    [[nodiscard]] bool createItem(const ItemRecord& item);
    [[nodiscard]] std::optional<ItemRecord> getItemById(std::uint64_t itemId) const;
    [[nodiscard]] bool addInventoryItem(const InventoryRecord& inventory);
    [[nodiscard]] std::vector<InventoryRecord> getInventoryByCharacter(std::uint64_t characterId) const;
    [[nodiscard]] bool updateInventoryQuantity(std::uint64_t inventoryId, std::uint32_t quantity);
    [[nodiscard]] bool deleteInventoryItem(std::uint64_t inventoryId);

    // ---- 活点地图 & 背包 ----
    [[nodiscard]] std::int32_t hasItem(std::uint64_t characterId, const std::string& itemName) const;
    [[nodiscard]] std::vector<PlayerInventoryDO> getInventoryPaged(
        std::uint64_t characterId, std::int32_t offset, std::int32_t limit) const;
    [[nodiscard]] int countInventoryItems(std::uint64_t characterId) const;

private:
    std::shared_ptr<DBConnection> connection_;
};

} // namespace arcane::database
