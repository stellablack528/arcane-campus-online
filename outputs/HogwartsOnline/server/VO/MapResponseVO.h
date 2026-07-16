#pragma once

#include <string>
#include <vector>

namespace arcane::vo {

/**
 * VO — 活点地图返回的单个玩家信息
 */
struct MapPlayerVO {
    std::string nickname;
    std::string house;
    std::string location;
    std::string statusEmoji;      // 🟢在线 / 🔴离线
};

/**
 * VO — 活点地图完整响应
 */
struct MapResponseVO {
    bool        success     = false;
    std::string message;
    std::string spellQuote;
    int         onlineCount = 0;
    std::vector<MapPlayerVO> players;
};

} // namespace arcane::vo
