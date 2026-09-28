#include "application/diagon_alley/DiagonAlleyCatalog.hpp"

#include "ui/I18n.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace arcane::application::diagon_alley {

namespace {

LocalizedText localizedText(const QJsonObject& object, const char* key)
{
    const auto value = object.value(QLatin1String(key)).toObject();
    return {value.value(QStringLiteral("zh")).toString(),
            value.value(QStringLiteral("en")).toString()};
}

QStringList stringList(const QJsonValue& value)
{
    QStringList result;
    for (const auto& item : value.toArray()) result.push_back(item.toString().toLower());
    return result;
}

bool isComplete(const LocalizedText& text)
{
    return !text.zh.trimmed().isEmpty() && !text.en.trimmed().isEmpty();
}

} // namespace

QString LocalizedText::current() const
{
    return arcane::ui::I18n::instance().language() == arcane::ui::I18n::Lang::Chinese ? zh : en;
}

bool DiagonAlleyCatalog::load()
{
    QFile file(QStringLiteral(":/data/diagon_alley.json"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const auto document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) return false;
    const auto root = document.object();
    if (root.value(QStringLiteral("schema_version")).toInt() != 1
        || root.value(QStringLiteral("chapter_id")).toString().trimmed().isEmpty()) {
        return false;
    }
    chapterTitle_ = localizedText(root, "chapter_title");

    const auto street = root.value(QStringLiteral("street")).toObject();
    streetTitle_ = localizedText(street, "title");
    streetIntro_ = localizedText(street, "intro");
    streetRevisit_ = localizedText(street, "revisit");
    completionText_ = localizedText(street, "completion");

    shops_.clear();
    for (const auto& shopValue : root.value(QStringLiteral("shops")).toArray()) {
        const auto object = shopValue.toObject();
        ShopDefinition shop;
        shop.id = object.value(QStringLiteral("id")).toString();
        shop.title = localizedText(object, "title");
        shop.keywords = stringList(object.value(QStringLiteral("keywords")));
        shop.intro = localizedText(object, "intro");
        shop.revisit = localizedText(object, "revisit");

        const auto npcObject = object.value(QStringLiteral("npc")).toObject();
        shop.npc.id = npcObject.value(QStringLiteral("id")).toString();
        shop.npc.name = localizedText(npcObject, "name");
        shop.npc.greeting = localizedText(npcObject, "greeting");
        shop.npc.warmGreeting = localizedText(npcObject, "warm_greeting");
        shop.npc.waryGreeting = localizedText(npcObject, "wary_greeting");
        shop.npc.talkText = localizedText(npcObject, "talk");
        shop.npc.followupText = localizedText(npcObject, "followup");
        shop.npc.ignoredText = localizedText(npcObject, "ignored");

        for (const auto& purchaseValue : object.value(QStringLiteral("purchases")).toArray()) {
            const auto purchaseObject = purchaseValue.toObject();
            PurchaseDefinition purchase;
            purchase.id = purchaseObject.value(QStringLiteral("id")).toString();
            purchase.label = localizedText(purchaseObject, "label");
            purchase.keywords = stringList(purchaseObject.value(QStringLiteral("keywords")));
            purchase.itemId = static_cast<std::uint64_t>(purchaseObject.value(QStringLiteral("item_id")).toInteger());
            purchase.price = static_cast<std::uint64_t>(purchaseObject.value(QStringLiteral("price")).toInteger());
            purchase.completionFlag = purchaseObject.value(QStringLiteral("completion_flag")).toString();
            purchase.successText = localizedText(purchaseObject, "success");
            shop.purchases.push_back(std::move(purchase));
        }
        shops_.push_back(std::move(shop));
    }

    if (!isComplete(chapterTitle_) || !isComplete(streetTitle_) || !isComplete(streetIntro_)
        || !isComplete(streetRevisit_) || !isComplete(completionText_) || shops_.isEmpty()) {
        shops_.clear();
        loaded_ = false;
        return false;
    }

    QSet<QString> shopIds;
    QSet<QString> npcIds;
    QSet<QString> purchaseIds;
    for (const auto& shop : shops_) {
        const bool validShop = !shop.id.isEmpty() && !shopIds.contains(shop.id)
            && isComplete(shop.title) && isComplete(shop.intro) && isComplete(shop.revisit)
            && !shop.keywords.isEmpty() && !shop.npc.id.isEmpty() && !npcIds.contains(shop.npc.id)
            && isComplete(shop.npc.name) && isComplete(shop.npc.greeting)
            && isComplete(shop.npc.warmGreeting) && isComplete(shop.npc.waryGreeting)
            && isComplete(shop.npc.talkText) && isComplete(shop.npc.followupText)
            && isComplete(shop.npc.ignoredText) && !shop.purchases.isEmpty();
        if (!validShop) {
            shops_.clear();
            loaded_ = false;
            return false;
        }
        shopIds.insert(shop.id);
        npcIds.insert(shop.npc.id);

        for (const auto& purchase : shop.purchases) {
            const bool validPurchase = !purchase.id.isEmpty() && !purchaseIds.contains(purchase.id)
                && isComplete(purchase.label) && !purchase.keywords.isEmpty()
                && purchase.itemId > 0 && !purchase.completionFlag.isEmpty()
                && isComplete(purchase.successText);
            if (!validPurchase) {
                shops_.clear();
                loaded_ = false;
                return false;
            }
            purchaseIds.insert(purchase.id);
        }
    }
    loaded_ = true;
    return loaded_;
}

const ShopDefinition* DiagonAlleyCatalog::findShop(const QString& id) const
{
    for (const auto& shop : shops_) if (shop.id == id) return &shop;
    return nullptr;
}

const ShopDefinition* DiagonAlleyCatalog::findShopByKeyword(const QString& normalizedText) const
{
    for (const auto& shop : shops_) {
        for (const auto& keyword : shop.keywords) {
            if (normalizedText.contains(keyword, Qt::CaseInsensitive)) return &shop;
        }
    }
    return nullptr;
}

const PurchaseDefinition* DiagonAlleyCatalog::findPurchase(const ShopDefinition& shop,
                                                            const QString& id) const
{
    for (const auto& purchase : shop.purchases) if (purchase.id == id) return &purchase;
    return nullptr;
}

const PurchaseDefinition* DiagonAlleyCatalog::findPurchaseByKeyword(
    const ShopDefinition& shop, const QString& normalizedText) const
{
    for (const auto& purchase : shop.purchases) {
        for (const auto& keyword : purchase.keywords) {
            if (normalizedText.contains(keyword, Qt::CaseInsensitive)) return &purchase;
        }
    }
    return nullptr;
}

} // namespace arcane::application::diagon_alley
