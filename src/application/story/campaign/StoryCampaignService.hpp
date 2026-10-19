#pragma once

#include "application/story/campaign/StoryChapterCatalog.hpp"

namespace arcane::application::story::campaign {

// Loads the built-in first-year campaign and exposes chapter copies to the
// controller. Runtime execution and database transactions remain in their
// existing dedicated services.
class StoryCampaignService final {
public:
    [[nodiscard]] bool loadBuiltIn(QStringList& errors);
    [[nodiscard]] bool isLoaded() const noexcept { return loaded_; }
    [[nodiscard]] bool contains(const QString& chapterId) const;
    [[nodiscard]] std::optional<ChapterDefinition> chapter(const QString& chapterId) const;
    [[nodiscard]] QStringList chapterIds() const;

private:
    StoryChapterCatalog catalog_;
    bool loaded_ = false;
};

} // namespace arcane::application::story::campaign
