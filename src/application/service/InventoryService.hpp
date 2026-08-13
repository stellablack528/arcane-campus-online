#pragma once

#include "application/dto/CampusDTO.hpp"
#include "application/vo/CampusVO.hpp"

#include <memory>

namespace arcane::database {
class InventoryDAO;
} // namespace arcane::database

namespace arcane::application::service {

class InventoryService final {
public:
    explicit InventoryService(std::shared_ptr<database::InventoryDAO> inventoryDao) noexcept;

    // Pull the character's inventory from the database and assemble a view-model list.
    [[nodiscard]] vo::InventoryListVO listInventory(const dto::InventoryListRequestDTO& request) const;

    // Action-based operations. When the DAO is available these look up the real item name;
    // otherwise they fall back to a small in-memory table for offline demos.
    [[nodiscard]] vo::OperationResultVO useItem(const dto::ItemActionRequestDTO& request) const;
    [[nodiscard]] vo::OperationResultVO inspectItem(const dto::ItemActionRequestDTO& request) const;
    [[nodiscard]] vo::OperationResultVO giftItem(const dto::ItemActionRequestDTO& request) const;

private:
    [[nodiscard]] std::string resolveItemName(const std::string& itemId) const;

    std::shared_ptr<database::InventoryDAO> inventoryDao_;
};

} // namespace arcane::application::service
