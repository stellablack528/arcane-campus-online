#include "application/diagon_alley/DiagonAlleyService.hpp"

#include "ui/I18n.hpp"

#include <algorithm>

namespace arcane::application::diagon_alley {

namespace {

bool isChinese()
{
    return arcane::ui::I18n::instance().language() == arcane::ui::I18n::Lang::Chinese;
}

QString localized(const char* chinese, const char* english)
{
    return isChinese() ? QString::fromUtf8(chinese) : QString::fromUtf8(english);
}

QString completedMarker(bool completed)
{
    return completed ? QStringLiteral("  ✓") : QString();
}

} // namespace

DiagonAlleyService::DiagonAlleyService(QObject* parent)
    : QObject(parent)
{
    (void)catalog_.load();
}

QStringList DiagonAlleyService::flags() const
{
    QStringList result(flags_.begin(), flags_.end());
    std::sort(result.begin(), result.end());
    return result;
}

void DiagonAlleyService::start(std::uint64_t characterId,
                               const QString& savedSceneId,
                               const QStringList& savedFlags,
                               const QHash<QString, int>& savedAffinities,
                               const QHash<QString, QStringList>& savedMemoryFlags)
{
    if (!catalog_.isLoaded() && !catalog_.load()) {
        emit narrativeProduced(localized("系统", "System"),
                               localized("对角巷内容包无法加载。", "The Diagon Alley content pack could not be loaded."));
        return;
    }

    characterId_ = characterId;
    active_ = true;
    flags_ = QSet<QString>(savedFlags.begin(), savedFlags.end());
    affinities_ = savedAffinities;
    npcMemoryFlags_.clear();
    for (auto it = savedMemoryFlags.cbegin(); it != savedMemoryFlags.cend(); ++it)
        npcMemoryFlags_[it.key()] = QSet<QString>(it.value().begin(), it.value().end());

    currentSceneId_ = catalog_.findShop(savedSceneId) ? savedSceneId : QStringLiteral("street");
    if (currentSceneId_ == QLatin1String("street")) {
        const bool firstVisit = !flags_.contains(QStringLiteral("visited_street"));
        flags_.insert(QStringLiteral("visited_street"));
        renderStreet(firstVisit);
    } else if (const auto* shop = catalog_.findShop(currentSceneId_)) {
        renderShop(*shop, true);
    }
    emitState();
}

bool DiagonAlleyService::submitText(const QString& text)
{
    if (!active_) return false;
    const auto action = parser_.parse(text, currentSceneId_, catalog_);
    if (action.kind == ActionKind::Unknown) {
        emit narrativeProduced(
            localized("旁白", "Narrator"),
            localized("你可以说“去丽痕书店”“和店主交谈”“购买课本”“回到街上”或“查看采购清单”。",
                      "Try “go to Flourish and Blotts”, “talk to the shopkeeper”, “buy the books”, “return to the street”, or “show my list”."));
        return true;
    }
    execute(action);
    return true;
}

void DiagonAlleyService::submitChoice(int index)
{
    if (!active_ || index < 0 || index >= currentChoices_.size()) return;
    const auto choice = currentChoices_.at(index);
    execute({choice.kind, choice.targetId});
}

void DiagonAlleyService::execute(const ParsedAction& action)
{
    switch (action.kind) {
    case ActionKind::Look:
        if (currentSceneId_ == QLatin1String("street")) renderStreet(false);
        else if (const auto* shop = catalog_.findShop(currentSceneId_)) renderShop(*shop, false);
        break;
    case ActionKind::EnterShop:
        enterShop(action.targetId);
        break;
    case ActionKind::ReturnToStreet:
        returnToStreet();
        break;
    case ActionKind::Talk:
        talkToShopkeeper();
        break;
    case ActionKind::Ignore:
        ignoreShopkeeper();
        break;
    case ActionKind::Buy:
        requestPurchase(action.targetId);
        break;
    case ActionKind::ShoppingList:
        showShoppingList();
        break;
    case ActionKind::Unknown:
        break;
    }
}

void DiagonAlleyService::renderStreet(bool firstVisit)
{
    currentSceneId_ = QStringLiteral("street");
    emit sceneHeaderChanged(catalog_.streetTitle());
    emit inputHintChanged(localized("描述你想前往的商店……", "Describe the shop you want to visit..."));
    emit narrativeProduced(localized("旁白", "Narrator"),
                           firstVisit ? catalog_.streetIntro() : catalog_.streetRevisit());
    updateChoicesForStreet();
}

void DiagonAlleyService::enterShop(const QString& shopId)
{
    const auto* shop = catalog_.findShop(shopId);
    if (!shop) return;
    currentSceneId_ = shopId;
    const bool enteringForFirstTime = !hasFlag(QStringLiteral("visited_%1").arg(shopId));
    flags_.insert(QStringLiteral("visited_%1").arg(shopId));
    renderShop(*shop, enteringForFirstTime);
    emitState();
}

void DiagonAlleyService::renderShop(const ShopDefinition& shop, bool entering)
{
    emit sceneHeaderChanged(shop.title.current());
    emit inputHintChanged(localized("你可以交谈、购买，或者返回街上……",
                                    "You can talk, buy something, or return to the street..."));
    emit narrativeProduced(localized("旁白", "Narrator"),
                           entering ? shop.intro.current() : shop.revisit.current());

    const int affinity = affinities_.value(shop.npc.id, 0);
    QString greeting = shop.npc.greeting.current();
    if (affinity > 0) greeting = shop.npc.warmGreeting.current();
    if (affinity < 0) greeting = shop.npc.waryGreeting.current();
    emit narrativeProduced(shop.npc.name.current(), greeting);
    updateChoicesForShop(shop);
}

void DiagonAlleyService::showShoppingList()
{
    const auto line = [this](const QString& flag, const char* zh, const char* en) {
        const QString mark = hasFlag(flag) ? QStringLiteral("✓") : QStringLiteral("○");
        return QStringLiteral("%1 %2").arg(mark, localized(zh, en));
    };
    const QString body = QStringLiteral("%1<br>%2<br>%3<br>%4<br>%5")
        .arg(line(QStringLiteral("wand"), "魔杖", "Wand"),
             line(QStringLiteral("books"), "课本与羽毛笔", "Books and quill"),
             line(QStringLiteral("robes"), "校服长袍", "School robes"),
             line(QStringLiteral("potions"), "魔药课用品", "Potion supplies"),
             line(QStringLiteral("pet"), "宠物伙伴", "Animal companion"));
    emit narrativeProduced(localized("采购清单", "Shopping List"), body);
}

void DiagonAlleyService::talkToShopkeeper()
{
    const auto* shop = catalog_.findShop(currentSceneId_);
    if (!shop) {
        emit narrativeProduced(localized("旁白", "Narrator"),
                               localized("街上的声音太多，你需要先走近某个人。",
                                         "There are too many voices in the street; approach someone first."));
        return;
    }
    auto& npcFlags = npcMemoryFlags_[shop->npc.id];
    const bool firstConversation = !npcFlags.contains(QStringLiteral("talked"));
    if (firstConversation) {
        npcFlags.insert(QStringLiteral("talked"));
        affinities_[shop->npc.id] = affinities_.value(shop->npc.id) + 1;
    }
    emit narrativeProduced(shop->npc.name.current(),
                           firstConversation ? shop->npc.talkText.current()
                                             : shop->npc.followupText.current());
    emit npcMemoryChanged(shop->npc.id, affinities_.value(shop->npc.id), memoryFlags(shop->npc.id));
    updateChoicesForShop(*shop);
}

void DiagonAlleyService::ignoreShopkeeper()
{
    const auto* shop = catalog_.findShop(currentSceneId_);
    if (!shop) return;
    auto& npcFlags = npcMemoryFlags_[shop->npc.id];
    if (!npcFlags.contains(QStringLiteral("ignored"))) {
        npcFlags.insert(QStringLiteral("ignored"));
        affinities_[shop->npc.id] = affinities_.value(shop->npc.id) - 1;
    }
    emit narrativeProduced(localized("旁白", "Narrator"), shop->npc.ignoredText.current());
    emit npcMemoryChanged(shop->npc.id, affinities_.value(shop->npc.id), memoryFlags(shop->npc.id));
    updateChoicesForShop(*shop);
}

void DiagonAlleyService::requestPurchase(const QString& purchaseId)
{
    const auto* shop = catalog_.findShop(currentSceneId_);
    if (!shop) return;
    const auto* purchase = catalog_.findPurchase(*shop, purchaseId);
    if (!purchase) return;
    if (hasFlag(purchase->completionFlag)) {
        emit narrativeProduced(shop->npc.name.current(),
                               localized("这一项已经替你包好了。", "That item is already wrapped for you."));
        return;
    }
    pendingPurchaseId_ = purchaseId;
    emit choicesPresented({});
    emit purchaseRequested(purchase->id, purchase->itemId, purchase->price);
}

void DiagonAlleyService::resolvePurchase(const QString& purchaseId,
                                         bool success,
                                         std::uint64_t remainingGold,
                                         const QString& message)
{
    if (purchaseId != pendingPurchaseId_) return;
    pendingPurchaseId_.clear();
    const auto* shop = catalog_.findShop(currentSceneId_);
    const auto* purchase = shop ? catalog_.findPurchase(*shop, purchaseId) : nullptr;
    if (!shop || !purchase) return;

    if (!success) {
        emit narrativeProduced(shop->npc.name.current(), message.toHtmlEscaped());
        updateChoicesForShop(*shop);
        return;
    }

    flags_.insert(purchase->completionFlag);
    emit narrativeProduced(shop->npc.name.current(), purchase->successText.current());
    emit narrativeProduced(localized("系统", "System"),
                           localized("剩余加隆：", "Galleons remaining: ")
                               + QString::number(remainingGold));
    emitState();
    maybeCompleteShopping();
    updateChoicesForShop(*shop);
}

void DiagonAlleyService::returnToStreet()
{
    if (currentSceneId_ == QLatin1String("street")) {
        renderStreet(false);
        return;
    }
    renderStreet(false);
    emitState();
}

void DiagonAlleyService::updateChoicesForStreet()
{
    currentChoices_.clear();
    QStringList labels;
    for (const auto& shop : catalog_.shops()) {
        bool completed = false;
        for (const auto& purchase : shop.purchases)
            completed = completed || hasFlag(purchase.completionFlag);
        labels << shop.title.current() + completedMarker(completed);
        currentChoices_.push_back({ActionKind::EnterShop, shop.id});
    }
    labels << localized("查看采购清单", "View shopping list");
    currentChoices_.push_back({ActionKind::ShoppingList, {}});
    emit choicesPresented(labels);
}

void DiagonAlleyService::updateChoicesForShop(const ShopDefinition& shop)
{
    currentChoices_.clear();
    QStringList labels;
    for (const auto& purchase : shop.purchases) {
        labels << purchase.label.current() + QStringLiteral(" · %1 G").arg(purchase.price)
                  + completedMarker(hasFlag(purchase.completionFlag));
        currentChoices_.push_back({ActionKind::Buy, purchase.id});
    }
    labels << localized("和店主交谈", "Talk to the shopkeeper");
    currentChoices_.push_back({ActionKind::Talk, {}});
    labels << localized("不回应，继续看商品", "Ignore them and browse");
    currentChoices_.push_back({ActionKind::Ignore, {}});
    labels << localized("回到街上", "Return to the street");
    currentChoices_.push_back({ActionKind::ReturnToStreet, {}});
    emit choicesPresented(labels);
}

void DiagonAlleyService::emitState()
{
    emit stateChanged(currentSceneId_, flags());
}

void DiagonAlleyService::maybeCompleteShopping()
{
    const bool complete = hasFlag(QStringLiteral("wand"))
        && hasFlag(QStringLiteral("books"))
        && hasFlag(QStringLiteral("robes"))
        && hasFlag(QStringLiteral("potions"))
        && hasFlag(QStringLiteral("pet"));
    if (!complete || hasFlag(QStringLiteral("shopping_complete"))) return;
    flags_.insert(QStringLiteral("shopping_complete"));
    emit narrativeProduced(localized("旁白", "Narrator"), catalog_.completionText());
    emitState();
}

QStringList DiagonAlleyService::memoryFlags(const QString& npcId) const
{
    const auto values = npcMemoryFlags_.value(npcId);
    QStringList result(values.begin(), values.end());
    std::sort(result.begin(), result.end());
    return result;
}

} // namespace arcane::application::diagon_alley
