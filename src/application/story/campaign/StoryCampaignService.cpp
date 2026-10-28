#include "application/story/campaign/StoryCampaignService.hpp"

#include "application/diagon_alley/DiagonAlleyCatalog.hpp"
#include "application/story/adapters/DiagonAlleyChapterAdapter.hpp"

#include <utility>

namespace arcane::application::story::campaign {

bool StoryCampaignService::loadBuiltIn(QStringList& errors)
{
    errors.clear();
    if (loaded_) return true;

    diagon_alley::DiagonAlleyCatalog diagonAlley;
    if (!diagonAlley.load()) {
        errors.push_back(QStringLiteral("The Diagon Alley content pack could not be loaded."));
        return false;
    }

    StoryChapterCatalog staged;
    adapters::DiagonAlleyChapterAdapter adapter;
    (void)staged.add(adapter.build(diagonAlley), errors);
    (void)staged.load(QStringLiteral(":/story/chapters/journey_to_hogwarts.json"), errors);
    (void)staged.load(QStringLiteral(":/story/chapters/sorting_ceremony.json"), errors);
    (void)staged.load(QStringLiteral(":/story/chapters/first_charms_lesson.json"), errors);
    (void)staged.load(QStringLiteral(":/story/chapters/turning_pages.json"), errors);
    errors.append(staged.validateTransitions());
    if (!errors.isEmpty()) return false;

    catalog_ = std::move(staged);
    loaded_ = true;
    return true;
}

bool StoryCampaignService::contains(const QString& chapterId) const
{
    return catalog_.find(chapterId) != nullptr;
}

std::optional<ChapterDefinition> StoryCampaignService::chapter(const QString& chapterId) const
{
    return catalog_.copy(chapterId);
}

QStringList StoryCampaignService::chapterIds() const
{
    return catalog_.chapterIds();
}

} // namespace arcane::application::story::campaign
