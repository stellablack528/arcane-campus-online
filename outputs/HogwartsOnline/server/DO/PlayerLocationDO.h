#pragma once

#include <cstdint>
#include <string>

namespace arcane::do_layer {

/**
 * DO (Data Object) — 与 player_location 表 1:1 映射
 *
 * 📖 DO 的规则：
 *   字段名 ≈ 数据库列名，类型对应，不含业务逻辑
 */
struct PlayerLocationDO {
    std::uint64_t characterId = 0;
    std::string    roomName;
    std::int32_t   xPos = 0;
    std::int32_t   yPos = 0;
    bool           isOnline = false;
    std::string    lastSeen;
};

} // namespace arcane::do_layer
