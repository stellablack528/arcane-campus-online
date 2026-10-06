#include "application/story/StoryRuntimeService.hpp"

#include "application/story/StoryContentValidator.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace arcane::application::story {

StoryRuntimeService::StoryRuntimeService(QObject* parent)
    : QObject(parent)
{
}

void StoryRuntimeService::setCommitter(DeltaCommitter committer)
{
    committer_ = std::move(committer);
}

bool StoryRuntimeService::start(ChapterDefinition chapter, StoryRuntimeState state)
{
    stop();
    const StoryContentValidator validator;
    const auto validation = validator.validate(chapter);
    if (!validation.isValid()) {
        emit runtimeError(QStringLiteral("Story content is invalid: %1")
                              .arg(validation.errors.join(QStringLiteral("; "))));
        return false;
    }

    if (state.chapterId.isEmpty()) state.chapterId = chapter.chapterId;
    if (state.chapterId != chapter.chapterId) {
        emit runtimeError(QStringLiteral("The saved state belongs to another chapter."));
        return false;
    }
    if (state.currentSceneId.isEmpty()) state.currentSceneId = chapter.entrySceneId;
    if (state.contentVersion == 0) state.contentVersion = chapter.contentVersion;
    if (state.contentVersion != chapter.contentVersion) {
        emit runtimeError(QStringLiteral("The saved story content version is incompatible."));
        return false;
    }

    QString reason;
    if (!engine_.canStart(chapter, state, &reason)) {
        emit runtimeError(reason);
        return false;
    }

    chapter_ = std::move(chapter);
    state_ = std::move(state);
    active_ = true;
    const auto entry = engine_.enterCurrentScene(*chapter_, state_);
    if (!commitAndPublish(entry)) {
        stop();
        return false;
    }
    emit inputHintChanged(language_ == StoryLanguage::Chinese
                              ? QStringLiteral("描述你想做的事……")
                              : QStringLiteral("Describe what you want to do..."));
    return true;
}

void StoryRuntimeService::stop()
{
    active_ = false;
    chapter_.reset();
    state_ = {};
    visibleChoices_.clear();
}

const StoryRuntimeState* StoryRuntimeService::state() const noexcept
{
    return active_ ? &state_ : nullptr;
}

QStringList StoryRuntimeService::visibleActionIds() const
{
    QStringList ids;
    ids.reserve(visibleChoices_.size());
    for (const auto& choice : visibleChoices_) ids.push_back(choice.actionId);
    return ids;
}

bool StoryRuntimeService::submitAction(const QString& actionId)
{
    if (!active_ || !chapter_) {
        emit runtimeError(QStringLiteral("No story chapter is active."));
        return false;
    }
    return commitAndPublish(engine_.execute(*chapter_, state_, actionId));
}

bool StoryRuntimeService::submitText(const QString& text)
{
    if (!active_ || !chapter_) {
        emit runtimeError(QStringLiteral("No story chapter is active."));
        return false;
    }
    const auto actionId = resolveActionId(text);
    if (!actionId) {
        emit runtimeError(language_ == StoryLanguage::Chinese
                              ? QStringLiteral("这个行动在当前场景中无法执行。")
                              : QStringLiteral("That action is not available in this scene."));
        return false;
    }
    return submitAction(*actionId);
}

bool StoryRuntimeService::submitChoice(int index)
{
    if (index < 0 || index >= visibleChoices_.size()) {
        emit runtimeError(QStringLiteral("The selected story choice is no longer available."));
        return false;
    }
    return submitAction(visibleChoices_.at(index).actionId);
}

bool StoryRuntimeService::commitAndPublish(StoryExecutionResult result)
{
    if (!result.success) {
        emit runtimeError(result.message);
        return false;
    }

    QString error;
    StoryRuntimeState committedState = state_;
    if (!engine_.applyDelta(committedState, result.delta, &error)) {
        emit runtimeError(error);
        return false;
    }
    if (state_.revision == std::numeric_limits<std::uint64_t>::max()) {
        emit runtimeError(QStringLiteral("The story revision limit has been reached."));
        return false;
    }
    if (committer_ && !committer_(*chapter_, state_, result, error)) {
        emit runtimeError(error.isEmpty() ? QStringLiteral("The story change could not be saved.")
                                          : error);
        return false;
    }

    committedState.revision = state_.revision + 1;
    committedState.contentVersion = chapter_->contentVersion;
    state_ = std::move(committedState);
    publishEvents(result.events);
    QStringList flags(state_.flags.begin(), state_.flags.end());
    flags.sort();
    emit runtimeStateChanged(state_.currentSceneId, flags);
    return true;
}

void StoryRuntimeService::publishEvents(const QVector<StoryEvent>& events)
{
    for (const auto& event : events) {
        switch (event.kind) {
        case StoryEventKind::SceneHeader:
            emit sceneHeaderChanged(resolve(event.text));
            break;
        case StoryEventKind::Narration:
        case StoryEventKind::NpcDialogue:
        case StoryEventKind::PlayerAction:
        case StoryEventKind::SystemNotice:
        case StoryEventKind::ChapterCompleted:
            emit narrativeProduced(resolve(event.speaker), resolve(event.text));
            break;
        case StoryEventKind::ChoicesChanged: {
            visibleChoices_ = event.choices;
            QStringList labels;
            labels.reserve(visibleChoices_.size());
            for (const auto& choice : visibleChoices_) labels.push_back(resolve(choice.label));
            emit choicesPresented(labels);
            break;
        }
        }
    }
}

QString StoryRuntimeService::resolve(const LocalizedStoryText& text) const
{
    return text.resolve(language_);
}

std::optional<QString> StoryRuntimeService::resolveActionId(const QString& text) const
{
    if (!chapter_) return std::nullopt;
    const QString input = normalize(text);
    if (input.isEmpty()) return std::nullopt;

    const auto actions = engine_.availableActions(*chapter_, state_);
    for (const auto* action : actions) {
        if (normalize(action->id) == input || normalize(resolve(action->label)) == input)
            return action->id;
        for (const auto& alias : action->aliases) {
            if (normalize(alias) == input) return action->id;
        }
    }

    const StoryActionDefinition* best = nullptr;
    qsizetype bestLength = 0;
    bool ambiguous = false;
    for (const auto* action : actions) {
        for (const auto& alias : action->aliases) {
            const QString normalizedAlias = normalize(alias);
            if (normalizedAlias.isEmpty() || !input.contains(normalizedAlias)) continue;
            if (normalizedAlias.size() > bestLength) {
                best = action;
                bestLength = normalizedAlias.size();
                ambiguous = false;
            } else if (normalizedAlias.size() == bestLength && best && best->id != action->id) {
                ambiguous = true;
            }
        }
    }
    return best && !ambiguous ? std::optional<QString>{best->id} : std::nullopt;
}

QString StoryRuntimeService::normalize(QString text)
{
    text = text.trimmed().toLower();
    static const QString punctuation = QStringLiteral("，。！？,.!?;；:：\"'“”‘’");
    for (const auto character : punctuation) text.remove(character);
    return text.simplified();
}

} // namespace arcane::application::story
