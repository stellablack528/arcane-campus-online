#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace arcane::vo {

/**
 * VO — 单个背包物品
 */
struct InventoryItemVO {
    std::uint64_t itemId   = 0;
    std::string   itemName;
    std::string   itemType;
    std::string   description;
    std::int32_t  quantity = 0;
    bool          usable   = false;
    std::string   icon;
};

/**
 * VO — 背包列表完整响应（含分页信息）
 */
struct InventoryListResponseVO {
    bool           success      = false;
    std::string    message;
    std::uint64_t  characterId  = 0;
    std::int32_t   totalItems   = 0;
    std::int32_t   currentPage  = 1;
    std::int32_t   pageSize     = 10;
    std::vector<InventoryItemVO> items;
};

} // namespace arcane::vo
