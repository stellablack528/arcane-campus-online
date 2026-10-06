#pragma once

#include "application/story/StoryEngine.hpp"

#include <QObject>
#include <QStringList>

#include <functional>
#include <optional>

namespace arcane::application::story {

// Stateful orchestration boundary between StoryEngine and the Qt narrative UI.
// A committer must accept the complete delta before any player-facing event is
// emitted. The next persistence phase can replace the default in-memory commit
// with one atomic database transaction without changing the UI contract.
class StoryRuntimeService final : public QObject {
    Q_OBJECT

public:
    using DeltaCommitter = std::function<bool(
        const ChapterDefinition& chapter,
        const StoryRuntimeState& currentState,
        const StoryExecutionResult& result,
        QString& error)>;

    explicit StoryRuntimeService(QObject* parent = nullptr);

    void setLanguage(StoryLanguage language) noexcept { language_ = language; }
    void setCommitter(DeltaCommitter committer);

    [[nodiscard]] bool start(ChapterDefinition chapter, StoryRuntimeState state);
    void stop();
    [[nodiscard]] bool isActive() const noexcept { return active_; }
    [[nodiscard]] const StoryRuntimeState* state() const noexcept;
    [[nodiscard]] QStringList visibleActionIds() const;

    [[nodiscard]] bool submitAction(const QString& actionId);
    [[nodiscard]] bool submitText(const QString& text);
    [[nodiscard]] bool submitChoice(int index);

signals:
    void narrativeProduced(const QString& speaker, const QString& htmlBody);
    void sceneHeaderChanged(const QString& title);
    void inputHintChanged(const QString& hint);
    void choicesPresented(const QStringList& labels);
    void runtimeStateChanged(const QString& sceneId, const QStringList& flags);
    void runtimeError(const QString& message);

private:
    [[nodiscard]] bool commitAndPublish(StoryExecutionResult result);
    void publishEvents(const QVector<StoryEvent>& events);
    [[nodiscard]] QString resolve(const LocalizedStoryText& text) const;
    [[nodiscard]] std::optional<QString> resolveActionId(const QString& text) const;
    static QString normalize(QString text);

    StoryEngine engine_;
    std::optional<ChapterDefinition> chapter_;
    StoryRuntimeState state_;
    StoryLanguage language_ = StoryLanguage::Chinese;
    DeltaCommitter committer_;
    QVector<StoryChoicePresentation> visibleChoices_;
    bool active_ = false;
};

} // namespace arcane::application::story
