#pragma once

#include "Service/InventoryService.h"
#include "Query/InventoryQuery.h"
#include "VO/InventoryListResponseVO.h"

#include <memory>

namespace arcane::controller {

/**
 * Controller — 玩家背包接口
 *
 * 🔗 GET /inventory/list?characterId={id}&page={page}&pageSize={size}
 */
class InventoryController {
public:
    explicit InventoryController(std::shared_ptr<arcane::service::InventoryService> service);

    [[nodiscard]] vo::InventoryListResponseVO listInventory(const query::InventoryQuery& q) const;

private:
    std::shared_ptr<arcane::service::InventoryService> service_;
};

} // namespace arcane::controller
