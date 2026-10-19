#include "application/story/adapters/DiagonAlleyChapterAdapter.hpp"

#include "application/diagon_alley/DiagonAlleyCatalog.hpp"

#include <utility>

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

StoryLineDefinition npcLine(const diagon_alley::NpcDefinition& npc,
                            const diagon_alley::LocalizedText& text,
                            QVector<StoryCondition> conditions = {})
{
    return {StoryLineKind::NpcDialogue, npc.id, storyText(npc.name), storyText(text),
            std::move(conditions)};
}

StoryLineDefinition narrationLine(const diagon_alley::LocalizedText& text)
{
    return {StoryLineKind::Narration, {}, {}, storyText(text), {}};
}

StoryLineDefinition purchaseNotice()
{
    return {StoryLineKind::SystemNotice, {}, {},
            {QStringLiteral("金币与背包已经更新。"),
             QStringLiteral("Your Galleons and inventory have been updated.")}, {}};
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

StoryLineDefinition checklistLine(const QString& flag,
                                  bool completed,
                                  const QString& chinese,
                                  const QString& english)
{
    const QString marker = completed ? QStringLiteral("✓") : QStringLiteral("○");
    return {StoryLineKind::SystemNotice, {}, {},
            {QStringLiteral("%1 %2").arg(marker, chinese),
             QStringLiteral("%1 %2").arg(marker, english)},
            {FlagCondition{flag, completed}}};
}

StoryActionDefinition shoppingListAction()
{
    StoryActionDefinition action;
    action.id = QStringLiteral("show_shopping_list");
    action.label = {QStringLiteral("查看采购清单"), QStringLiteral("Show shopping list")};
    action.playerText = action.label;
    action.aliases = {QStringLiteral("采购清单"), QStringLiteral("购物清单"),
                      QStringLiteral("查看清单"), QStringLiteral("shopping list"),
                      QStringLiteral("show list")};
    const auto appendItem = [&](const QString& flag,
                                const QString& chinese,
                                const QString& english) {
        action.responseLines.push_back(checklistLine(flag, true, chinese, english));
        action.responseLines.push_back(checklistLine(flag, false, chinese, english));
    };
    appendItem(QStringLiteral("wand"), QStringLiteral("魔杖"), QStringLiteral("Wand"));
    appendItem(QStringLiteral("books"), QStringLiteral("课本与羽毛笔"),
               QStringLiteral("Books and quill"));
    appendItem(QStringLiteral("robes"), QStringLiteral("校服长袍"),
               QStringLiteral("School robes"));
    appendItem(QStringLiteral("potions"), QStringLiteral("魔药课用品"),
               QStringLiteral("Potion supplies"));
    appendItem(QStringLiteral("pet"), QStringLiteral("宠物伙伴"),
               QStringLiteral("Animal companion"));
    return action;
}

StoryActionDefinition departForKingsCrossAction()
{
    StoryActionDefinition action;
    action.id = QStringLiteral("depart_for_kings_cross");
    action.label = {QStringLiteral("前往国王十字车站"),
                    QStringLiteral("Leave for King's Cross")};
    action.playerText = {QStringLiteral("你确认清单上的物品已经备齐，提起行李前往国王十字车站。"),
                         QStringLiteral("With every item checked off, you lift your luggage and leave for King's Cross.")};
    action.aliases = {QStringLiteral("前往国王十字车站"), QStringLiteral("去车站"),
                      QStringLiteral("离开对角巷"), QStringLiteral("kings cross"),
                      QStringLiteral("leave diagon alley")};
    action.conditions = {FlagCondition{QStringLiteral("shopping_complete"), true}};
    action.effects = {TransitionChapterEffect{QStringLiteral("journey_to_hogwarts")}};
    action.responseLines = {{
        StoryLineKind::Narration, {}, {},
        {QStringLiteral("店铺的灯光被你留在身后。下一段路从麻瓜车站的钟声开始。"),
         QStringLiteral("The shop lights fall behind you. The next part of the journey begins beneath a Muggle station clock.")},
        {}}};
    return action;
}

StoryActionDefinition purchaseAction(const diagon_alley::NpcDefinition& npc,
                                     const diagon_alley::PurchaseDefinition& purchase)
{
    StoryActionDefinition action;
    action.id = QStringLiteral("buy_%1").arg(purchase.id);
    action.label = {
        QStringLiteral("%1 · %2 G").arg(purchase.label.zh).arg(purchase.price),
        QStringLiteral("%1 · %2 G").arg(purchase.label.en).arg(purchase.price),
    };
    action.playerText = storyText(purchase.label);
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
    action.responseLines = {npcLine(npc, purchase.successText), purchaseNotice()};
    return action;
}

StoryActionDefinition memoryAction(const diagon_alley::NpcDefinition& npc,
                                   const QString& kind,
                                   const LocalizedStoryText& label,
                                   bool firstTime,
                                   int affinityChange,
                                   StoryLineDefinition response)
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
    action.responseLines.push_back(std::move(response));
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
    chapter.completionText = storyText(catalog.completionStoryText());
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
    street.revisitNarration = storyText(catalog.streetRevisitText());
    for (const auto& shop : catalog.shops()) street.actions.push_back(enterShopAction(shop));
    street.actions.push_back(shoppingListAction());
    street.actions.push_back(departForKingsCrossAction());
    chapter.scenes.push_back(std::move(street));

    for (const auto& shop : catalog.shops()) {
        SceneDefinition scene;
        scene.id = shop.id;
        scene.title = storyText(shop.title);
        scene.narration = storyText(shop.intro);
        scene.revisitNarration = storyText(shop.revisit);
        scene.actorIds = {shop.npc.id};
        scene.arrivalLines = {
            npcLine(shop.npc, shop.npc.greeting,
                    {AffinityAtLeastCondition{shop.npc.id, 0},
                     AffinityAtMostCondition{shop.npc.id, 0}}),
            npcLine(shop.npc, shop.npc.warmGreeting,
                    {AffinityAtLeastCondition{shop.npc.id, 1}}),
            npcLine(shop.npc, shop.npc.waryGreeting,
                    {AffinityAtMostCondition{shop.npc.id, -1}}),
        };
        for (const auto& purchase : shop.purchases)
            scene.actions.push_back(purchaseAction(shop.npc, purchase));
        scene.actions.push_back(memoryAction(
            shop.npc, QStringLiteral("talk"),
            {QStringLiteral("和店主交谈"), QStringLiteral("Talk to the shopkeeper")}, true, 1,
            npcLine(shop.npc, shop.npc.talkText)));
        scene.actions.push_back(memoryAction(
            shop.npc, QStringLiteral("talk"),
            {QStringLiteral("继续交谈"), QStringLiteral("Talk again")}, false, 0,
            npcLine(shop.npc, shop.npc.followupText)));
        scene.actions.push_back(memoryAction(
            shop.npc, QStringLiteral("ignore"),
            {QStringLiteral("不回应，继续看商品"), QStringLiteral("Ignore them and browse")},
            true, -1, narrationLine(shop.npc.ignoredText)));
        scene.actions.push_back(memoryAction(
            shop.npc, QStringLiteral("ignore"),
            {QStringLiteral("继续看商品"), QStringLiteral("Keep browsing")}, false, 0,
            narrationLine(shop.npc.ignoredText)));
        scene.actions.push_back(returnAction(shop));
        chapter.scenes.push_back(std::move(scene));
    }

    return chapter;
}

void DiagonAlleyChapterAdapter::seedFromLegacyProgress(
    StoryRuntimeState& state,
    const ChapterDefinition& chapter,
    const QString& sceneId,
    const QStringList& flags) const
{
    state.chapterId = chapter.chapterId;
    state.contentVersion = chapter.contentVersion;
    state.currentSceneId = chapter.findScene(sceneId) ? sceneId : chapter.entrySceneId;
    for (const auto& flag : flags) state.flags.insert(flag);
    if (state.flags.contains(QStringLiteral("shopping_complete"))) {
        state.chapterCompleted = true;
        state.completedChapters.insert(chapter.chapterId);
    }
}

} // namespace arcane::application::story::adapters
