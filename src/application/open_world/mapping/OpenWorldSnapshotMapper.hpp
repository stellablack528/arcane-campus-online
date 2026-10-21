#pragma once

#include "application/open_world/model/OpenWorldSnapshot.hpp"

#include <optional>
#include <string>

namespace arcane::application::story {
struct StoryRuntimeState;
}

namespace arcane::application::open_world {

struct OpenWorldSnapshotMappingResult {
    std::optional<OpenWorldSnapshot> snapshot;
    std::string error;

    [[nodiscard]] bool success() const noexcept
    {
        return snapshot.has_value();
    }
};

// Converts an already restored story state into the provider-neutral boundary
// model. It never mutates the source and performs no database access.
class OpenWorldSnapshotMapper final {
public:
    [[nodiscard]] static OpenWorldSnapshotMappingResult map(
        const story::StoryRuntimeState& state);
};

} // namespace arcane::application::open_world
