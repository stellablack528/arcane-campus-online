#pragma once

#include "application/diagon_alley/DiagonAlleyActionParser.hpp"
#include "application/diagon_alley/DiagonAlleyCatalog.hpp"

#include <QHash>
#include <QObject>
#include <QSet>
#include <QStringList>

#include <cstdint>

namespace arcane::application::diagon_alley {

class DiagonAlleyService final : public QObject {
    Q_OBJECT

public:
    explicit DiagonAlleyService(QObject* parent = nullptr);

    [[nodiscard]] bool isActive() const noexcept { return active_; }
    [[nodiscard]] QString currentSceneId() const { return currentSceneId_; }
    [[nodiscard]] QStringList flags() const;

    void start(std::uint64_t characterId,
               const QString& savedSceneId = QStringLiteral("street"),
               const QStringList& savedFlags = {},
               const QHash<QString, int>& savedAffinities = {},
               const QHash<QString, QStringList>& savedMemoryFlags = {});
    [[nodiscard]] bool submitText(const QString& text);
    void submitChoice(int index);
    void resolvePurchase(const QString& purchaseId,
                         bool success,
                         std::uint64_t remainingGold,
                         const QString& message);

signals:
    void narrativeProduced(const QString& speaker, const QString& htmlBody);
    void sceneHeaderChanged(const QString& title);
    void inputHintChanged(const QString& hint);
    void choicesPresented(const QStringList& labels);
    void stateChanged(const QString& sceneId, const QStringList& flags);
    void npcMemoryChanged(const QString& npcId, int affinity, const QStringList& memoryFlags);
    void purchaseRequested(const QString& purchaseId,
                           std::uint64_t itemId,
                           std::uint64_t price);

private:
    struct ChoiceAction {
        ActionKind kind = ActionKind::Unknown;
        QString targetId;
    };

    void execute(const ParsedAction& action);
    void renderStreet(bool firstVisit);
    void enterShop(const QString& shopId);
    void renderShop(const ShopDefinition& shop, bool entering);
    void showShoppingList();
    void talkToShopkeeper();
    void ignoreShopkeeper();
    void requestPurchase(const QString& purchaseId);
    void returnToStreet();
    void updateChoicesForStreet();
    void updateChoicesForShop(const ShopDefinition& shop);
    void emitState();
    void maybeCompleteShopping();
    [[nodiscard]] QStringList memoryFlags(const QString& npcId) const;
    [[nodiscard]] bool hasFlag(const QString& flag) const { return flags_.contains(flag); }

    DiagonAlleyCatalog catalog_;
    DiagonAlleyActionParser parser_;
    bool active_ = false;
    std::uint64_t characterId_ = 0;
    QString currentSceneId_ = QStringLiteral("street");
    QString pendingPurchaseId_;
    QSet<QString> flags_;
    QHash<QString, int> affinities_;
    QHash<QString, QSet<QString>> npcMemoryFlags_;
    QVector<ChoiceAction> currentChoices_;
};

} // namespace arcane::application::diagon_alley
