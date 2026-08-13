#include "application/service/MapService.hpp"

#include "InventoryDAO.hpp"
#include "LocationDAO.hpp"

#include <algorithm>
#include <array>
#include <random>
#include <utility>

namespace arcane::application::service {

namespace {
// Canonical name of the Marauder's Map item stored in the items table.
constexpr const char* kMaraudersMapItemName = "Marauder's Map";

// Classic incantations spoken when the map is activated.
const std::array<const char*, 6> kSpellQuotes = {
    "I solemnly swear that I am up to no good.",
    "Messrs. Moony, Wormtail, Padfoot, and Prongs are proud to present the Marauder's Map.",
    "The map never lies. It reveals every secret passage and hidden step.",
    "Mischief managed... but first, let us see who wanders tonight.",
    "Point me! The map shows all within these walls.",
    "Those who wander are not always lost — the map proves it.",
};

const char* houseEmoji(const std::string& house)
{
    if (house == "Gryffindor") return "🦁";
    if (house == "Slytherin") return "🐍";
    if (house == "Hufflepuff") return "🦡";
    if (house == "Ravenclaw") return "🦅";
    return "✨";
}
} // namespace

MapService::MapService(std::shared_ptr<database::InventoryDAO> inventoryDao,
                       std::shared_ptr<database::LocationDAO> locationDao) noexcept
    : inventoryDao_(std::move(inventoryDao))
    , locationDao_(std::move(locationDao))
{
}

vo::MapResultVO MapService::useMaraudersMap(const dto::MapUseRequestDTO& request) const
{
    if (request.characterId == 0) {
        return {false, "A character must be selected before using the map.", {}, 0, {}};
    }

    if (!ownsMaraudersMap(request.characterId)) {
        return {false,
                "You do not possess the Marauder's Map. Seek it in the shelves of Filch's office.",
                {},
                0,
                {}};
    }

    if (!locationDao_) {
        return {false, "The map is blank — location tracking is unavailable.", {}, 0, {}};
    }

    const auto records = request.roomId == 0
        ? locationDao_->getOnlinePlayers()
        : locationDao_->getOnlinePlayersInRoom(request.roomId);

    vo::MapResultVO result;
    result.success = true;
    result.spellQuote = pickSpellQuote();
    result.onlineCount = static_cast<std::uint32_t>(records.size());
    result.message = records.empty()
        ? "The map is quiet. No one stirs within the grounds."
        : "The parchment shimmers, revealing " + std::to_string(records.size())
              + (records.size() == 1 ? " wanderer." : " wanderers.");

    result.players.reserve(records.size());
    for (const auto& record : records) {
        vo::MapPlayerVO player;
        player.characterId = record.characterId;
        player.nickname = record.nickname;
        player.house = record.house;
        player.roomName = record.roomName;
        player.posX = record.posX;
        player.posY = record.posY;
        player.statusEmoji = houseEmoji(record.house);
        result.players.push_back(std::move(player));
    }
    return result;
}

bool MapService::ownsMaraudersMap(std::uint64_t characterId) const
{
    if (!inventoryDao_) {
        return false;
    }
    // Walk the character's inventory and resolve each item definition to check the name.
    // A dedicated DAO method (e.g. hasItemByName) could short-circuit this in the future.
    const auto inventory = inventoryDao_->getInventoryByCharacter(characterId);
    for (const auto& slot : inventory) {
        const auto item = inventoryDao_->getItemById(slot.itemId);
        if (item && item->itemName == kMaraudersMapItemName && slot.quantity > 0) {
            return true;
        }
    }
    return false;
}

std::string MapService::pickSpellQuote()
{
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<std::size_t> distribution(0, kSpellQuotes.size() - 1);
    return kSpellQuotes[distribution(generator)];
}

} // namespace arcane::application::service
