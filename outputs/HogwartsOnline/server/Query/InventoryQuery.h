#pragma once

#include <cstdint>

namespace arcane::query {

/**
 * Query — 玩家背包接口的请求参数
 *
 * 📖 GET /inventory/list?characterId=1&page=1&pageSize=10
 *                        └──────┬──────┘└──┬──┘└───┬────┘
 *                           Query 参数全在这里
 */
struct InventoryQuery {
    std::uint64_t characterId = 0;
    std::int32_t  page        = 1;
    std::int32_t  pageSize    = 10;
};

} // namespace arcane::query
