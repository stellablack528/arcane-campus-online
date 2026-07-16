#pragma once

#include "Query/InventoryQuery.h"
#include "DAO/InventoryDAO.hpp"
#include "VO/InventoryListResponseVO.h"

#include <cstdint>
#include <memory>

namespace arcane::service {

/**
 * Service — 玩家背包业务逻辑
 *
 * 业务流程：
 *   1. 接收 Query 参数（characterId, page, pageSize）
 *   2. 调 DAO 查总数 + 分页数据
 *   3. DO → VO 转换
 *   4. 组装分页信息返回
 */
class InventoryService {
public:
    explicit InventoryService(std::shared_ptr<arcane::database::InventoryDAO> inventoryDao);

    [[nodiscard]] vo::InventoryListResponseVO listInventory(const query::InventoryQuery& q) const;

private:
    std::shared_ptr<arcane::database::InventoryDAO> inventoryDao_;
};

} // namespace arcane::service
