#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace arcane::application::router {

// Command routing for the future TCP network layer.
//
// NOTE: The current build is a single-process Qt desktop app where the UI talks
// to CampusController directly through Qt signals/slots, so this router is not
// wired into the request path yet. Once a TCP (or other socket) gateway is added,
// incoming command strings should be translated here and dispatched to the
// corresponding CampusController slot.
enum class ClientCommandType {
    Login,
    Chat,
    Move,
    Course,
    ItemUse,
    ItemInspect,
    ItemGift,
    UseMaraudersMap,
    RefreshInventory,
};

struct ClientCommand {
    ClientCommandType type;
    std::string payload;
};

class ClientCommandRouter final {
public:
    [[nodiscard]] static std::optional<ClientCommand> fromName(std::string_view commandName,
                                                                std::string payload);
};

} // namespace arcane::application::router
