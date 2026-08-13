#include "application/service/InventoryService.hpp"

#include "InventoryDAO.hpp"

#include <unordered_map>
#include <utility>

namespace arcane::application::service {

namespace {
// Fallback names used when no DAO is wired in (offline demo mode).
const std::unordered_map<std::string, std::string> fallbackItemNames{
    {"wand", "Wand"},
    {"chocolate_frog", "Chocolate Frog"},
    {"butterbeer", "Butterbeer"},
    {"potion", "Potion"},
    {"quill", "Quill"},
    {"galleon", "Galleons"},
};

std::string tryParseItemId(const std::string& itemId)
{
    // itemId may be a numeric database id or a legacy string key. Keep both working.
    return itemId;
}
} // namespace

InventoryService::InventoryService(std::shared_ptr<database::InventoryDAO> inventoryDao) noexcept
    : inventoryDao_(std::move(inventoryDao))
{
}

vo::InventoryListVO InventoryService::listInventory(const dto::InventoryListRequestDTO& request) const
{
    if (request.characterId == 0) {
        return {false, "A character must be selected before listing the inventory.", 0, {}};
    }
    if (!inventoryDao_) {
        return {false, "Inventory storage is not configured.", 0, {}};
    }

    const auto slots = inventoryDao_->getInventoryByCharacter(request.characterId);

    vo::InventoryListVO response;
    response.success = true;
    response.characterId = request.characterId;
    response.message = "Inventory loaded.";
    response.totalItems = static_cast<std::uint32_t>(slots.size());
    response.items.reserve(slots.size());

    for (const auto& slot : slots) {
        const auto item = inventoryDao_->getItemById(slot.itemId);
        if (!item) {
            continue;
        }
        response.items.push_back(vo::InventoryItemVO{
            item->itemId,
            item->itemName,
            item->itemType,
            item->description,
            slot.quantity,
            item->stackable,
        });
    }
    return response;
}

vo::OperationResultVO InventoryService::useItem(const dto::ItemActionRequestDTO& request) const
{
    const auto name = resolveItemName(request.itemId);
    if (name.empty()) {
        return {false, "That item is unavailable."};
    }
    return {true, "You used " + name + "."};
}

vo::OperationResultVO InventoryService::inspectItem(const dto::ItemActionRequestDTO& request) const
{
    const auto name = resolveItemName(request.itemId);
    if (name.empty()) {
        return {false, "That item is unavailable."};
    }
    return {true, "You inspected " + name + "."};
}

vo::OperationResultVO InventoryService::giftItem(const dto::ItemActionRequestDTO& request) const
{
    if (request.npcId.empty()) {
        return {false, "Choose an NPC before giving a gift."};
    }
    const auto name = resolveItemName(request.itemId);
    if (name.empty()) {
        return {false, "That item is unavailable."};
    }
    return {true, "You offered " + name + " to " + request.npcId + "."};
}

std::string InventoryService::resolveItemName(const std::string& itemId) const
{
    const auto key = tryParseItemId(itemId);

    // 1. Try the DAO first (numeric id).
    if (inventoryDao_) {
        if (!key.empty()) {
            for (std::size_t i = 0; i < key.size(); ++i) {
                if (key[i] < '0' || key[i] > '9') {
                    break;
                }
                if (i + 1 == key.size()) {
                    try {
                        const auto id = std::stoull(key);
                        const auto item = inventoryDao_->getItemById(id);
                        if (item) {
                            return item->itemName;
                        }
                    } catch (...) {
                        // fall through to fallback table
                    }
                }
            }
        }
    }

    // 2. Fallback to the demo table (string keys like "wand").
    const auto iterator = fallbackItemNames.find(key);
    return iterator == fallbackItemNames.end() ? std::string{} : iterator->second;
}

} // namespace arcane::application::service
