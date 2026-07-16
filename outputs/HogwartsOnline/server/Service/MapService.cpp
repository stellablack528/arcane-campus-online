#include "MapService.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace arcane::service {

MapService::MapService(
    std::shared_ptr<arcane::database::LocationDAO> locationDao,
    std::shared_ptr<arcane::database::InventoryDAO> inventoryDao)
    : locationDao_(std::move(locationDao)), inventoryDao_(std::move(inventoryDao)) {}

vo::MapResponseVO MapService::useMaraudersMap(std::uint64_t characterId) const {
    // Step 1: 检查道具
    if (inventoryDao_->hasItem(characterId, "Marauder's Map") == 0) {
        return { false, "You don't have the Marauder's Map!", "...The parchment remains blank.", 0 };
    }

    // Step 2: 查询在线玩家
    const auto onlineLocations = locationDao_->findAllOnline();

    // Step 3: DO → VO 转换
    vo::MapResponseVO response;
    response.success     = true;
    response.message     = "Mischief managed!";
    response.spellQuote  = randomSpellQuote();
    response.onlineCount = static_cast<int>(onlineLocations.size());

    for (const auto& loc : onlineLocations) {
        vo::MapPlayerVO player;
        player.nickname    = "Player#" + std::to_string(loc.characterId);
        player.location    = loc.roomName + " (" + std::to_string(loc.xPos) + ", " + std::to_string(loc.yPos) + ")";
        player.statusEmoji = loc.isOnline ? "\xF0\x9F\x9F\xA2" : "\xF0\x9F\x94\xB4"; // 🟢 / 🔴
        response.players.push_back(player);
    }
    return response;
}

std::string MapService::randomSpellQuote() {
    static const std::string quotes[] = {
        "Mr. Moony presents his compliments to Professor Snape and begs him to keep his abnormally large nose out of other people's business.",
        "Mr. Prongs agrees with Mr. Moony and would like to add that Professor Snape is an ugly git.",
        "Mr. Padfoot would like to register his astonishment that an idiot like that ever became a Professor.",
        "Mr. Wormtail bids Professor Snape good day, and advises him to wash his hair, the slimeball.",
        "\"I solemnly swear that I am up to no good.\"",
        "\"Mischief managed!\""
    };
    static bool seeded = false;
    if (!seeded) { std::srand(static_cast<unsigned>(std::time(nullptr))); seeded = true; }
    return quotes[std::rand() % (sizeof(quotes) / sizeof(quotes[0]))];
}

} // namespace arcane::service
