#include "application/router/ClientCommandRouter.hpp"

#include <array>

namespace arcane::application::router {

std::optional<ClientCommand> ClientCommandRouter::fromName(std::string_view commandName,
                                                            std::string payload)
{
    static constexpr std::array mappings{
        std::pair{"login", ClientCommandType::Login},
        std::pair{"chat", ClientCommandType::Chat},
        std::pair{"move", ClientCommandType::Move},
        std::pair{"course", ClientCommandType::Course},
        std::pair{"item_use", ClientCommandType::ItemUse},
        std::pair{"item_inspect", ClientCommandType::ItemInspect},
        std::pair{"item_gift", ClientCommandType::ItemGift},
        std::pair{"use_marauders_map", ClientCommandType::UseMaraudersMap},
        std::pair{"refresh_inventory", ClientCommandType::RefreshInventory},
    };
    for (const auto& [name, type] : mappings) {
        if (commandName == name) {
            return ClientCommand{type, std::move(payload)};
        }
    }
    return std::nullopt;
}

} // namespace arcane::application::router
