#include "InventoryController.h"

namespace arcane::controller {

InventoryController::InventoryController(std::shared_ptr<arcane::service::InventoryService> service)
    : service_(std::move(service)) {}

vo::InventoryListResponseVO InventoryController::listInventory(const query::InventoryQuery& q) const {
    return service_->listInventory(q);
}

} // namespace arcane::controller
