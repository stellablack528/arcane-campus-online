#pragma once

#include "application/story/StoryTypes.hpp"

#include <QHash>
#include <QStringList>

#include <optional>

namespace arcane::application::story::campaign {

// Owns validated chapter definitions and checks cross-chapter transition targets.
// It contains no persistence or UI code, so campaign content stays testable.
class StoryChapterCatalog final {
public:
    [[nodiscard]] bool add(ChapterDefinition chapter, QStringList& errors);
    [[nodiscard]] bool load(const QString& resourcePath, QStringList& errors);
    [[nodiscard]] const ChapterDefinition* find(const QString& chapterId) const;
    [[nodiscard]] std::optional<ChapterDefinition> copy(const QString& chapterId) const;
    [[nodiscard]] QStringList chapterIds() const;
    [[nodiscard]] QStringList validateTransitions() const;
    [[nodiscard]] bool isEmpty() const noexcept { return chapters_.isEmpty(); }

private:
    QHash<QString, ChapterDefinition> chapters_;
};

} // namespace arcane::application::story::campaign
