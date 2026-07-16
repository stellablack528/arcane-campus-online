#pragma once

#include "DAO/LocationDAO.hpp"
#include "DAO/InventoryDAO.hpp"
#include "VO/MapResponseVO.h"

#include <cstdint>
#include <memory>
#include <string>

namespace arcane::service {

/**
 * Service — 活点地图业务逻辑
 *
 * 📖 编排业务流程：
 *   1. 检查玩家有没有地图道具
 *   2. 查询所有在线玩家位置
 *   3. DO → VO 转换
 */
class MapService {
public:
    MapService(std::shared_ptr<arcane::database::LocationDAO> locationDao,
               std::shared_ptr<arcane::database::InventoryDAO> inventoryDao);

    [[nodiscard]] vo::MapResponseVO useMaraudersMap(std::uint64_t characterId) const;

private:
    std::shared_ptr<arcane::database::LocationDAO> locationDao_;
    std::shared_ptr<arcane::database::InventoryDAO> inventoryDao_;
    [[nodiscard]] static std::string randomSpellQuote();
};

} // namespace arcane::service
