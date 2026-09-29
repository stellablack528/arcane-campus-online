#pragma once

#include "application/story/StoryTypes.hpp"

#include <QString>
#include <QVector>

namespace arcane::application::story {

class StoryEngine final {
public:
    [[nodiscard]] bool canStart(const ChapterDefinition& chapter,
                                const StoryRuntimeState& state,
                                QString* reason = nullptr) const;
    [[nodiscard]] QVector<const StoryActionDefinition*> availableActions(
        const ChapterDefinition& chapter,
        const StoryRuntimeState& state) const;
    [[nodiscard]] StoryExecutionResult execute(const ChapterDefinition& chapter,
                                               const StoryRuntimeState& state,
                                               const QString& actionId) const;
    [[nodiscard]] bool applyDelta(StoryRuntimeState& state,
                                  const StoryDelta& delta,
                                  QString* error = nullptr) const;

private:
    [[nodiscard]] bool conditionsSatisfied(const QVector<StoryCondition>& conditions,
                                           const StoryRuntimeState& state,
                                           QString* reason = nullptr) const;
};

} // namespace arcane::application::story
