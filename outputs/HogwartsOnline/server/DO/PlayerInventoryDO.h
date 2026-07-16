#pragma once

#include <cstdint>
#include <string>

namespace arcane::do_layer {

/**
 * DO — player_inventory + items JOIN 结果
 */
struct PlayerInventoryDO {
    std::uint64_t inventoryId = 0;
    std::uint64_t characterId = 0;
    std::uint64_t itemId      = 0;
    std::string   itemName;
    std::string   itemType;
    std::string   description;
    std::int32_t  quantity  = 0;
    bool          stackable = false;
};

} // namespace arcane::do_layer
