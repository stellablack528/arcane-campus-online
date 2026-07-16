#include "InventoryDAO.hpp"
#include <utility>

namespace arcane::database {

InventoryDAO::InventoryDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection)) {}

bool InventoryDAO::createItem(const ItemRecord& item) {
    return connection_->execute(
        "INSERT INTO items (item_name, item_type, description, stackable) VALUES (?, ?, ?, ?)",
        {item.itemName, item.itemType, item.description, item.stackable ? "1" : "0"});
}

std::optional<ItemRecord> InventoryDAO::getItemById(std::uint64_t itemId) const {
    const auto result = connection_->query("SELECT * FROM items WHERE item_id = ?", {std::to_string(itemId)});
    static_cast<void>(result);
    return std::nullopt;
}

bool InventoryDAO::addInventoryItem(const InventoryRecord& inventory) {
    return connection_->execute(
        "INSERT INTO inventories (character_id, item_id, quantity) VALUES (?, ?, ?)",
        {std::to_string(inventory.characterId), std::to_string(inventory.itemId), std::to_string(inventory.quantity)});
}

std::vector<InventoryRecord> InventoryDAO::getInventoryByCharacter(std::uint64_t characterId) const {
    const auto result = connection_->query("SELECT * FROM inventories WHERE character_id = ?", {std::to_string(characterId)});
    static_cast<void>(result);
    return {};
}

bool InventoryDAO::updateInventoryQuantity(std::uint64_t inventoryId, std::uint32_t quantity) {
    return connection_->execute("UPDATE inventories SET quantity = ? WHERE inventory_id = ?",
                                {std::to_string(quantity), std::to_string(inventoryId)});
}

bool InventoryDAO::deleteInventoryItem(std::uint64_t inventoryId) {
    return connection_->execute("DELETE FROM inventories WHERE inventory_id = ?", {std::to_string(inventoryId)});
}

// ---- 新增 ----

std::int32_t InventoryDAO::hasItem(std::uint64_t characterId, const std::string& itemName) const {
    const auto result = connection_->query(
        "SELECT quantity FROM inventories i JOIN items it ON i.item_id = it.item_id "
        "WHERE i.character_id = ? AND it.item_name = ?",
        {std::to_string(characterId), itemName});
    if (!result || result->rows.empty()) return 0;
    auto it = result->rows[0].find("quantity");
    return (it != result->rows[0].end()) ? std::stoi(it->second) : 0;
}

std::vector<PlayerInventoryDO> InventoryDAO::getInventoryPaged(
    std::uint64_t characterId, std::int32_t offset, std::int32_t limit) const {
    const auto result = connection_->query(
        "SELECT inv.inventory_id, inv.character_id, inv.item_id, "
        "it.item_name, it.item_type, it.description, it.stackable, inv.quantity "
        "FROM inventories inv JOIN items it ON inv.item_id = it.item_id "
        "WHERE inv.character_id = ? ORDER BY it.item_type, it.item_name "
        "LIMIT ? OFFSET ?",
        {std::to_string(characterId), std::to_string(limit), std::to_string(offset)});
    std::vector<PlayerInventoryDO> items;
    if (!result) return items;
    for (const auto& row : result->rows) {
        PlayerInventoryDO item{};
        auto get = [&](const std::string& k) { auto it = row.find(k); return (it != row.end()) ? it->second : ""; };
        item.inventoryId = std::stoull(get("inventory_id"));
        item.characterId = std::stoull(get("character_id"));
        item.itemId      = std::stoull(get("item_id"));
        item.itemName    = get("item_name");
        item.itemType    = get("item_type");
        item.description = get("description");
        item.quantity    = std::stoi(get("quantity"));
        item.stackable   = (get("stackable") == "1");
        items.push_back(item);
    }
    return items;
}

int InventoryDAO::countInventoryItems(std::uint64_t characterId) const {
    const auto result = connection_->query(
        "SELECT COUNT(*) as cnt FROM inventories WHERE character_id = ?", {std::to_string(characterId)});
    if (!result || result->rows.empty()) return 0;
    auto it = result->rows[0].find("cnt");
    return (it != result->rows[0].end()) ? std::stoi(it->second) : 0;
}

} // namespace arcane::database
