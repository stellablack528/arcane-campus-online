#include "InventoryDAO.hpp"

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
    static_cast<void>(result);
    return std::nullopt;
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
    static_cast<void>(result);
    return {};
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
