#pragma once

#include "application/story/StoryTypes.hpp"

#include <QStringList>

namespace arcane::application::diagon_alley {
class DiagonAlleyCatalog;
}

namespace arcane::application::story::adapters {

// Production content seam: translate the existing v1 Diagon Alley content
// into the generic engine model while the legacy service remains test-only.
class DiagonAlleyChapterAdapter final {
public:
    [[nodiscard]] ChapterDefinition build(
        const diagon_alley::DiagonAlleyCatalog& catalog) const;
    void seedFromLegacyProgress(StoryRuntimeState& state,
                                const ChapterDefinition& chapter,
                                const QString& sceneId,
                                const QStringList& flags) const;
};

} // namespace arcane::application::story::adapters
