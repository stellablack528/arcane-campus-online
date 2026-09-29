#pragma once

#include "application/story/StoryTypes.hpp"

namespace arcane::application::diagon_alley {
class DiagonAlleyCatalog;
}

namespace arcane::application::story::adapters {

// Phase-two migration seam: translate the existing v1 Diagon Alley content
// into the generic engine model without changing the production runtime yet.
class DiagonAlleyChapterAdapter final {
public:
    [[nodiscard]] ChapterDefinition build(
        const diagon_alley::DiagonAlleyCatalog& catalog) const;
};

} // namespace arcane::application::story::adapters
