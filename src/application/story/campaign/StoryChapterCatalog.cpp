#include "application/story/campaign/StoryChapterCatalog.hpp"

#include "application/story/StoryContentLoader.hpp"
#include "application/story/StoryContentValidator.hpp"

#include <algorithm>

namespace arcane::application::story::campaign {

bool StoryChapterCatalog::add(ChapterDefinition chapter, QStringList& errors)
{
    const StoryContentValidator validator;
    const auto validation = validator.validate(chapter);
    if (!validation.isValid()) {
        for (const auto& error : validation.errors)
            errors.push_back(QStringLiteral("%1: %2").arg(chapter.chapterId, error));
        return false;
    }
    if (chapters_.contains(chapter.chapterId)) {
        errors.push_back(QStringLiteral("Duplicate campaign chapter '%1'.").arg(chapter.chapterId));
        return false;
    }
    chapters_.insert(chapter.chapterId, std::move(chapter));
    return true;
}

bool StoryChapterCatalog::load(const QString& resourcePath, QStringList& errors)
{
    const StoryContentLoader loader;
    auto loaded = loader.loadFile(resourcePath);
    if (!loaded.success()) {
        for (const auto& error : loaded.errors) errors.push_back(error);
        return false;
    }
    return add(std::move(*loaded.chapter), errors);
}

const ChapterDefinition* StoryChapterCatalog::find(const QString& chapterId) const
{
    const auto it = chapters_.constFind(chapterId);
    return it == chapters_.cend() ? nullptr : &it.value();
}

std::optional<ChapterDefinition> StoryChapterCatalog::copy(const QString& chapterId) const
{
    const auto* chapter = find(chapterId);
    return chapter ? std::optional<ChapterDefinition>{*chapter} : std::nullopt;
}

QStringList StoryChapterCatalog::chapterIds() const
{
    QStringList ids = chapters_.keys();
    ids.sort();
    return ids;
}

QStringList StoryChapterCatalog::validateTransitions() const
{
    QStringList errors;
    for (auto chapterIt = chapters_.cbegin(); chapterIt != chapters_.cend(); ++chapterIt) {
        for (const auto& scene : chapterIt->scenes) {
            for (const auto& action : scene.actions) {
                for (const auto& effect : action.effects) {
                    const auto* transition = std::get_if<TransitionChapterEffect>(&effect);
                    if (!transition || chapters_.contains(transition->chapterId)) continue;
                    errors.push_back(
                        QStringLiteral("Chapter '%1', action '%2' transitions to missing chapter '%3'.")
                            .arg(chapterIt.key(), action.id, transition->chapterId));
                }
            }
        }
    }
    return errors;
}

} // namespace arcane::application::story::campaign
