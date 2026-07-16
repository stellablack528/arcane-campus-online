#pragma once

#include <cstdint>
#include <string>

namespace arcane::query {

/**
 * Query — 活点地图接口的请求参数
 *
 * 📖 Query 是什么？
 *   GET /map/use-marauders-map?characterId=1
 *                              └─────────┘ 这就是 Query 参数
 *
 * ⚠️ 目前只有 characterId，后续可以扩展 spell 等参数
 */
struct MapUseQuery {
    std::uint64_t characterId = 0;
};

} // namespace arcane::query
