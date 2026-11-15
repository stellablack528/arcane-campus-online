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

std::optional<ChapterDefinition> StoryCampaignService::chapterForEra(const QString& chapterId,
                                                                      const QString& eraId) const
{
    auto result = chapter(chapterId);
    if (!result || eraId == QLatin1String("harry")) return result;
    const bool parents = eraId == QLatin1String("parents");
    const QString classmateZh = parents ? QStringLiteral("艾达·芬奇") : QStringLiteral("伊芙琳·摩尔");
    const QString classmateEn = parents ? QStringLiteral("Ada Finch") : QStringLiteral("Evelyn Moore");
    const QString professorZh = parents ? QStringLiteral("梅里克教授") : QStringLiteral("艾尔德里奇教授");
    const QString professorEn = parents ? QStringLiteral("Professor Merrick") : QStringLiteral("Professor Eldridge");
    auto rewrite = [&](QString& value) {
        value.replace(QStringLiteral("罗温·维尔"), classmateZh);
        value.replace(QStringLiteral("Rowan Vale"), classmateEn);
        value.replace(QStringLiteral("罗温"), parents ? QStringLiteral("艾达") : QStringLiteral("伊芙琳"));
        value.replace(QStringLiteral("Rowan"), parents ? QStringLiteral("Ada") : QStringLiteral("Evelyn"));
        value.replace(QStringLiteral("弗立维教授"), professorZh);
        value.replace(QStringLiteral("Professor Flitwick"), professorEn);
        value.replace(QStringLiteral("弗立维"), parents ? QStringLiteral("梅里克") : QStringLiteral("艾尔德里奇"));
        value.replace(QStringLiteral("Flitwick"), parents ? QStringLiteral("Merrick") : QStringLiteral("Eldridge"));
    };
    auto text = [&](LocalizedStoryText& value) { rewrite(value.zh); rewrite(value.en); };
    text(result->title);
    text(result->completionText);
    for (auto& scene : result->scenes) {
        text(scene.title); text(scene.narration); text(scene.revisitNarration);
        for (auto& line : scene.arrivalLines) { text(line.speaker); text(line.text); }
        for (auto& action : scene.actions) {
            text(action.label); text(action.playerText);
            for (auto& alias : action.aliases) rewrite(alias);
            for (auto& line : action.responseLines) { text(line.speaker); text(line.text); }
        }
    }
    return result;
}

QStringList StoryCampaignService::chapterIds() const
{
    return catalog_.chapterIds();
}

} // namespace arcane::application::story::campaign
