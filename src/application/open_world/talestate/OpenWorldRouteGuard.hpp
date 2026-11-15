#pragma once

#include <string>

namespace arcane::application::open_world::talestate {

struct RouteDecision {
    bool allowed = false;
    std::string reason;
};

// TaleState checks the author-approved route before the campus service changes
// the live session. It never writes to the game's database or network state.
class OpenWorldRouteGuard final {
public:
    [[nodiscard]] static RouteDecision check(const std::string& currentSceneId,
                                             const std::string& destinationSceneId);
};

} // namespace arcane::application::open_world::talestate
