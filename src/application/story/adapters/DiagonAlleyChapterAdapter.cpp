#include "application/story/adapters/DiagonAlleyChapterAdapter.hpp"

#include "application/diagon_alley/DiagonAlleyCatalog.hpp"

namespace arcane::application::story::adapters {

namespace {

LocalizedStoryText storyText(const diagon_alley::LocalizedText& text)
{
    return {text.zh, text.en};
}

QStringList aliasesWith(const QStringList& source,
                        std::initializer_list<QString> additions)
{
    QStringList result = source;
    for (const auto& addition : additions) {
        if (!result.contains(addition, Qt::CaseInsensitive)) result.push_back(addition);
    }
    return result;
}

StoryActionDefinition enterShopAction(const diagon_alley::ShopDefinition& shop)
{
    StoryActionDefinition action;
    action.id = QStringLiteral("enter_%1").arg(shop.id);
    action.label = shop.title.zh.isEmpty() ? LocalizedStoryText{} : storyText(shop.title);
    action.aliases = shop.keywords;
    action.effects = {
        SetFlagEffect{QStringLiteral("visited_%1").arg(shop.id), true},
        MoveToSceneEffect{shop.id},
    };
    return action;
}

StoryActionDefinition purchaseAction(const diagon_alley::PurchaseDefinition& purchase)
{
    StoryActionDefinition action;
    action.id = QStringLiteral("buy_%1").arg(purchase.id);
    action.label = storyText(purchase.label);
    action.aliases = purchase.keywords;
    action.conditions = {
        FlagCondition{purchase.completionFlag, false},
        GoldAtLeastCondition{purchase.price},
    };
    action.effects = {
        ChangeGoldEffect{-static_cast<std::int64_t>(purchase.price)},
        ChangeItemEffect{purchase.itemId, 1},
        SetFlagEffect{purchase.completionFlag, true},
    };
    return action;
}

StoryActionDefinition memoryAction(const diagon_alley::NpcDefinition& npc,
                                   const QString& kind,
                                   const LocalizedStoryText& label,
                                   bool firstTime,
                                   int affinityChange)
{
    StoryActionDefinition action;
    action.id = QStringLiteral("%1_%2_%3")
                    .arg(kind, npc.id, firstTime ? QStringLiteral("first")
                                                : QStringLiteral("repeat"));
    action.label = label;
    action.aliases = kind == QLatin1String("talk")
        ? aliasesWith({}, {QStringLiteral("交谈"), QStringLiteral("说话"),
                           QStringLiteral("talk"), QStringLiteral("speak")})
        : aliasesWith({}, {QStringLiteral("不理"), QStringLiteral("忽略"),
                           QStringLiteral("ignore"), QStringLiteral("browse")});
    action.conditions = {
        NpcMemoryCondition{npc.id, kind == QLatin1String("talk")
                                       ? QStringLiteral("talked")
                                       : QStringLiteral("ignored"),
                           !firstTime},
    };
    const QString memoryKey = kind == QLatin1String("talk")
        ? QStringLiteral("talked") : QStringLiteral("ignored");
    action.effects.push_back(SetNpcMemoryEffect{npc.id, memoryKey, true});
    if (firstTime) action.effects.push_back(ChangeAffinityEffect{npc.id, affinityChange});
    return action;
}

StoryActionDefinition returnAction(const diagon_alley::ShopDefinition& shop)
{
    StoryActionDefinition action;
    action.id = QStringLiteral("return_%1_to_street").arg(shop.id);
    action.label = {QStringLiteral("回到街上"), QStringLiteral("Return to the street")};
    action.aliases = {QStringLiteral("回街上"), QStringLiteral("返回"),
                      QStringLiteral("return"), QStringLiteral("back")};
    action.effects = {MoveToSceneEffect{QStringLiteral("street")}};
    return action;
}

} // namespace

ChapterDefinition DiagonAlleyChapterAdapter::build(
    const diagon_alley::DiagonAlleyCatalog& catalog) const
{
    ChapterDefinition chapter;
    chapter.schemaVersion = 2;
    chapter.chapterId = QStringLiteral("diagon_alley_shopping");
    chapter.contentVersion = 1;
    chapter.entrySceneId = QStringLiteral("street");
    chapter.title = storyText(catalog.chapterTitleText());
    chapter.completionFlag = QStringLiteral("shopping_complete");
    chapter.allowActionsAfterCompletion = true;
    chapter.completionConditions = {
        FlagCondition{QStringLiteral("wand"), true},
        FlagCondition{QStringLiteral("books"), true},
        FlagCondition{QStringLiteral("robes"), true},
        FlagCondition{QStringLiteral("potions"), true},
        FlagCondition{QStringLiteral("pet"), true},
    };

    SceneDefinition street;
    street.id = QStringLiteral("street");
    street.title = storyText(catalog.streetTitleText());
    street.narration = storyText(catalog.streetIntroText());
    for (const auto& shop : catalog.shops()) street.actions.push_back(enterShopAction(shop));
    chapter.scenes.push_back(std::move(street));

    for (const auto& shop : catalog.shops()) {
        SceneDefinition scene;
        scene.id = shop.id;
        scene.title = storyText(shop.title);
        scene.narration = storyText(shop.intro);
        scene.actorIds = {shop.npc.id};
        for (const auto& purchase : shop.purchases)
            scene.actions.push_back(purchaseAction(purchase));
        scene.actions.push_back(memoryAction(
            shop.npc, QStringLiteral("talk"),
            {QStringLiteral("和店主交谈"), QStringLiteral("Talk to the shopkeeper")}, true, 1));
        scene.actions.push_back(memoryAction(
            shop.npc, QStringLiteral("talk"),
            {QStringLiteral("继续交谈"), QStringLiteral("Talk again")}, false, 0));
        scene.actions.push_back(memoryAction(
            shop.npc, QStringLiteral("ignore"),
            {QStringLiteral("不回应，继续看商品"), QStringLiteral("Ignore them and browse")},
            true, -1));
        scene.actions.push_back(memoryAction(
            shop.npc, QStringLiteral("ignore"),
            {QStringLiteral("继续看商品"), QStringLiteral("Keep browsing")}, false, 0));
        scene.actions.push_back(returnAction(shop));
        chapter.scenes.push_back(std::move(scene));
    }

    return chapter;
}

} // namespace arcane::application::story::adapters
