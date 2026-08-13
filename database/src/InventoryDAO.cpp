#include "InventoryDAO.hpp"
#include "RecordMapper.hpp"

#include <utility>

namespace arcane::database {

InventoryDAO::InventoryDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

bool InventoryDAO::createItem(const ItemRecord& item)
{
    return connection_->execute(
        "INSERT INTO items (item_name, item_type, description, stackable) VALUES (?, ?, ?, ?)",
        {item.itemName, item.itemType, item.description, item.stackable ? "1" : "0"});
}

std::optional<ItemRecord> InventoryDAO::getItemById(std::uint64_t itemId) const
{
    const auto result = connection_->query("SELECT * FROM items WHERE item_id = ?", {std::to_string(itemId)});
    if (!result || result->rows.empty()) {
        return std::nullopt;
    }
    const auto& row = result->rows.front();
    return ItemRecord{detail::integer<std::uint64_t>(row, "item_id"), detail::value(row, "item_name"),
                      detail::value(row, "item_type"), detail::value(row, "description"),
                      detail::boolean(row, "stackable")};
}

bool InventoryDAO::addInventoryItem(const InventoryRecord& inventory)
{
    return connection_->execute(
        "INSERT INTO inventories (character_id, item_id, quantity) VALUES (?, ?, ?)",
        {std::to_string(inventory.characterId),
         std::to_string(inventory.itemId),
         std::to_string(inventory.quantity)});
}

std::vector<InventoryRecord> InventoryDAO::getInventoryByCharacter(std::uint64_t characterId) const
{
    const auto result = connection_->query("SELECT * FROM inventories WHERE character_id = ?",
                                           {std::to_string(characterId)});
    std::vector<InventoryRecord> inventory;
    if (!result) {
        return inventory;
    }
    inventory.reserve(result->rows.size());
    for (const auto& row : result->rows) {
        inventory.push_back(InventoryRecord{detail::integer<std::uint64_t>(row, "inventory_id"),
                                            detail::integer<std::uint64_t>(row, "character_id"),
                                            detail::integer<std::uint64_t>(row, "item_id"),
                                            detail::integer<std::uint32_t>(row, "quantity")});
    }
    return inventory;
}

bool InventoryDAO::updateInventoryQuantity(std::uint64_t inventoryId, std::uint32_t quantity)
{
    return connection_->execute("UPDATE inventories SET quantity = ? WHERE inventory_id = ?",
                                {std::to_string(quantity), std::to_string(inventoryId)});
}

bool InventoryDAO::deleteInventoryItem(std::uint64_t inventoryId)
{
    return connection_->execute("DELETE FROM inventories WHERE inventory_id = ?",
                                {std::to_string(inventoryId)});
}

} // namespace arcane::database
