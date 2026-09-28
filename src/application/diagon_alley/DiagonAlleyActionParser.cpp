#include "application/diagon_alley/DiagonAlleyActionParser.hpp"

#include "application/diagon_alley/DiagonAlleyCatalog.hpp"

namespace arcane::application::diagon_alley {

namespace {

bool containsAny(const QString& text, std::initializer_list<const char*> words)
{
    for (const auto* word : words) {
        if (text.contains(QString::fromUtf8(word), Qt::CaseInsensitive)) return true;
    }
    return false;
}

} // namespace

QString DiagonAlleyActionParser::normalize(QString input)
{
    input = input.trimmed().toLower();
    static const QString punctuation = QStringLiteral("，。！？,.!?;；:：\"'“”‘’");
    for (const auto character : punctuation) input.remove(character);
    return input.simplified();
}

ParsedAction DiagonAlleyActionParser::parse(const QString& input,
                                            const QString& currentSceneId,
                                            const DiagonAlleyCatalog& catalog) const
{
    const QString text = normalize(input);
    if (text.isEmpty()) return {};

    if (containsAny(text, {"清单", "采购进度", "还缺什么", "list", "status"}))
        return {ActionKind::ShoppingList, {}};
    if (containsAny(text, {"回街上", "返回街道", "离开商店", "出去", "back", "leave"}))
        return {ActionKind::ReturnToStreet, {}};
    if (containsAny(text, {"无视", "不理", "走开", "ignore"}))
        return {ActionKind::Ignore, {}};
    if (containsAny(text, {"交谈", "说话", "问问", "打招呼", "talk", "hello", "speak"}))
        return {ActionKind::Talk, {}};
    if (currentSceneId == QLatin1String("street")) {
        if (const auto* shop = catalog.findShopByKeyword(text))
            return {ActionKind::EnterShop, shop->id};
        if (containsAny(text, {"看看", "观察", "环顾", "look", "inspect"}))
            return {ActionKind::Look, {}};
        return {};
    }

    const auto* shop = catalog.findShop(currentSceneId);
    if (!shop) return {};
    if (const auto* purchase = catalog.findPurchaseByKeyword(*shop, text)) {
        if (containsAny(text, {"买", "购买", "要这个", "选择", "领养", "buy", "choose", "adopt"})
            || shop->purchases.size() == 1) {
            return {ActionKind::Buy, purchase->id};
        }
    }
    if (containsAny(text, {"买", "购买", "结账", "buy"}) && shop->purchases.size() == 1)
        return {ActionKind::Buy, shop->purchases.first().id};
    if (containsAny(text, {"看看", "观察", "环顾", "look", "inspect"}))
        return {ActionKind::Look, {}};
    return {};
}

} // namespace arcane::application::diagon_alley
