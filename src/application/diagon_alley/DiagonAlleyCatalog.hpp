#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

#include <cstdint>

namespace arcane::application::diagon_alley {

struct LocalizedText {
    QString zh;
    QString en;
    [[nodiscard]] QString current() const;
};

struct PurchaseDefinition {
    QString id;
    LocalizedText label;
    QStringList keywords;
    std::uint64_t itemId = 0;
    std::uint64_t price = 0;
    QString completionFlag;
    LocalizedText successText;
};

struct NpcDefinition {
    QString id;
    LocalizedText name;
    LocalizedText greeting;
    LocalizedText warmGreeting;
    LocalizedText waryGreeting;
    LocalizedText talkText;
    LocalizedText followupText;
    LocalizedText ignoredText;
};

struct ShopDefinition {
    QString id;
    LocalizedText title;
    QStringList keywords;
    LocalizedText intro;
    LocalizedText revisit;
    NpcDefinition npc;
    QVector<PurchaseDefinition> purchases;
};

class DiagonAlleyCatalog final {
public:
    [[nodiscard]] bool load();
    [[nodiscard]] bool isLoaded() const noexcept { return loaded_; }
    [[nodiscard]] const QVector<ShopDefinition>& shops() const noexcept { return shops_; }
    [[nodiscard]] const ShopDefinition* findShop(const QString& id) const;
    [[nodiscard]] const ShopDefinition* findShopByKeyword(const QString& normalizedText) const;
    [[nodiscard]] const PurchaseDefinition* findPurchase(const ShopDefinition& shop,
                                                         const QString& id) const;
    [[nodiscard]] const PurchaseDefinition* findPurchaseByKeyword(const ShopDefinition& shop,
                                                                  const QString& normalizedText) const;

    [[nodiscard]] QString chapterTitle() const { return chapterTitle_.current(); }
    [[nodiscard]] QString streetTitle() const { return streetTitle_.current(); }
    [[nodiscard]] QString streetIntro() const { return streetIntro_.current(); }
    [[nodiscard]] QString streetRevisit() const { return streetRevisit_.current(); }
    [[nodiscard]] QString completionText() const { return completionText_.current(); }

    // The migration adapter needs both languages without depending on the
    // currently selected UI language. The existing localized accessors remain
    // unchanged for the legacy runtime.
    [[nodiscard]] const LocalizedText& chapterTitleText() const noexcept { return chapterTitle_; }
    [[nodiscard]] const LocalizedText& streetTitleText() const noexcept { return streetTitle_; }
    [[nodiscard]] const LocalizedText& streetIntroText() const noexcept { return streetIntro_; }
    [[nodiscard]] const LocalizedText& streetRevisitText() const noexcept { return streetRevisit_; }
    [[nodiscard]] const LocalizedText& completionStoryText() const noexcept { return completionText_; }

private:
    bool loaded_ = false;
    LocalizedText chapterTitle_;
    LocalizedText streetTitle_;
    LocalizedText streetIntro_;
    LocalizedText streetRevisit_;
    LocalizedText completionText_;
    QVector<ShopDefinition> shops_;
};

} // namespace arcane::application::diagon_alley
