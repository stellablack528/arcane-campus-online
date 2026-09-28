#pragma once

#include <QString>

namespace arcane::application::diagon_alley {

class DiagonAlleyCatalog;
struct ShopDefinition;

enum class ActionKind {
    Unknown,
    Look,
    EnterShop,
    ReturnToStreet,
    Talk,
    Ignore,
    Buy,
    ShoppingList,
};

struct ParsedAction {
    ActionKind kind = ActionKind::Unknown;
    QString targetId;
};

class DiagonAlleyActionParser final {
public:
    [[nodiscard]] ParsedAction parse(const QString& input,
                                     const QString& currentSceneId,
                                     const DiagonAlleyCatalog& catalog) const;

private:
    [[nodiscard]] static QString normalize(QString input);
};

} // namespace arcane::application::diagon_alley
