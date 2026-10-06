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
    // Produces the initial/resumed scene presentation and a delta that records
    // the visit. Callers persist the delta before rendering the events.
    [[nodiscard]] StoryExecutionResult enterCurrentScene(
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
    void appendLineEvents(QVector<StoryEvent>& destination,
                          const QVector<StoryLineDefinition>& lines,
                          const StoryRuntimeState& state,
                          const QString& sceneId) const;
    void appendSceneEvents(QVector<StoryEvent>& destination,
                           const SceneDefinition& scene,
                           const StoryRuntimeState& state,
                           bool firstVisit) const;
    [[nodiscard]] StoryEvent choicesEvent(const ChapterDefinition& chapter,
                                          const StoryRuntimeState& state) const;
};

} // namespace arcane::application::story
