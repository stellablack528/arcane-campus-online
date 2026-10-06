#pragma once

#include "application/story/StoryEngine.hpp"

#include <QString>

#include <cstdint>
#include <memory>

namespace arcane::database {
class DBConnection;
}

namespace arcane::application::story::persistence {

struct StoryLoadResult {
    bool success = false;
    bool found = false;
    StoryRuntimeState state;
    QString error;
};

// Database adapter for the generic story runtime. It restores a complete
// engine state and commits every table touched by one StoryDelta inside one
// transaction. The domain engine remains SQL-free.
class StoryPersistenceService final {
public:
    explicit StoryPersistenceService(
        std::shared_ptr<database::DBConnection> connection) noexcept;

    [[nodiscard]] StoryLoadResult load(const ChapterDefinition& chapter,
                                       std::uint64_t characterId) const;
    [[nodiscard]] bool commit(const ChapterDefinition& chapter,
                              const StoryRuntimeState& currentState,
                              const StoryExecutionResult& result,
                              QString& error) const;

private:
    std::shared_ptr<database::DBConnection> connection_;
};

} // namespace arcane::application::story::persistence
