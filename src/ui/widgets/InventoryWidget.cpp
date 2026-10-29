#include "ui/widgets/InventoryWidget.h"

#include "ui/I18n.hpp"

#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <array>

namespace {

const char* itemNameKey(std::uint64_t id)
{
    switch (id) {
    case 1: return "item.wand";
    case 2: return "item.chocolate";
    case 3: return "item.butterbeer";
    case 4: return "item.potion";
    case 5: return "item.quill";
    case 6: return "item.galleon";
    case 7: return "item.map";
    case 8: return "item.owl";
    case 9: return "item.cat";
    case 10: return "item.toad";
    case 11: return "item.robe";
    case 12: return "item.books";
    case 13: return "item.potionkit";
    default: return nullptr;
    }
}

int categoryFor(const arcane::application::vo::InventoryItemVO& item)
{
    if (item.itemType == "gift" || item.itemType == "affection_gift") return 3;
    if (item.itemType == "food" || item.itemType == "drink") return 0;
    if (item.itemId == 7 || item.itemType == "pet" || item.itemType == "currency"
        || item.itemType == "special") return 2;
    if (item.itemType == "equipment" || item.itemType == "tool"
        || item.itemType == "school_supply" || item.itemType == "consumable") return 1;
    return 2;
}

} // namespace

InventoryWidget::InventoryWidget(QWidget *parent) : QWidget(parent)
{
    setObjectName("InventoryWidget");
    buildUi();
    retranslateUi();
    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) { retranslateUi(); });
}

void InventoryWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("PanelTitle");
    m_list = new QTreeWidget(this);
    m_list->setObjectName("InventoryList");
    m_list->setHeaderHidden(true);
    m_list->setIndentation(15);
    m_list->setUniformRowHeights(false);
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    m_list->header()->setStretchLastSection(true);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_list, 1);

    connect(m_list, &QTreeWidget::itemDoubleClicked, this,
            [this](QTreeWidgetItem *item, int) {
        if (item && item->parent() && !itemId(item).isEmpty())
            emit itemUseRequested(itemId(item));
    });
    connect(m_list, &QTreeWidget::customContextMenuRequested, this,
            [this](const QPoint &pos) {
        auto *item = m_list->itemAt(pos);
        if (!item || !item->parent() || itemId(item).isEmpty()) return;
        QMenu menu(this);
        auto *inspect = menu.addAction(TR("item.inspect"));
        auto *gift = menu.addAction(TR("item.gift"));
        auto *chosen = menu.exec(m_list->viewport()->mapToGlobal(pos));
        if (chosen == inspect) emit itemInspectRequested(itemId(item));
        else if (chosen == gift)
            emit itemGiftRequested(itemId(item), QStringLiteral("npc_hermione"));
    });
}

void InventoryWidget::retranslateUi()
{
    m_titleLabel->setText(TR("title.inventory"));
    m_list->clear();
    constexpr std::array<const char*, 4> categories{
        "inventory.food", "inventory.magic", "inventory.special", "inventory.gifts"};
    std::array<QTreeWidgetItem*, 4> sections{};
    std::array<int, 4> counts{};
    for (int i = 0; i < 4; ++i) {
        sections[i] = new QTreeWidgetItem(m_list);
        sections[i]->setFlags(Qt::ItemIsEnabled);
        QFont font = sections[i]->font(0);
        font.setBold(true);
        sections[i]->setFont(0, font);
    }
    for (const auto& item : m_items) {
        const int category = categoryFor(item);
        ++counts[category];
        const char* key = itemNameKey(item.itemId);
        const QString name = key ? TRQ(QString::fromLatin1(key))
                                 : QString::fromStdString(item.itemName);
        auto *row = new QTreeWidgetItem(sections[category]);
        row->setText(0, QStringLiteral("%1  ×%2").arg(name).arg(item.quantity));
        row->setData(0, Qt::UserRole, QString::number(item.itemId));
        QString tooltip = QStringLiteral("%1\n%2 ×%3")
                              .arg(name, TR("inventory.quantity"))
                              .arg(item.quantity);
        if (arcane::ui::I18n::instance().language() == arcane::ui::I18n::Lang::English
            && !item.description.empty())
            tooltip += QLatin1Char('\n') + QString::fromStdString(item.description);
        row->setToolTip(0, tooltip);
    }
    for (int i = 0; i < 4; ++i) {
        sections[i]->setText(0, QStringLiteral("%1  ·  %2")
                                    .arg(TRQ(QString::fromLatin1(categories[i])))
                                    .arg(counts[i]));
        if (counts[i] == 0) {
            auto *empty = new QTreeWidgetItem(sections[i]);
            empty->setText(0, TR("inventory.empty"));
            empty->setFlags(Qt::NoItemFlags);
        }
        sections[i]->setExpanded(true);
    }
}

void InventoryWidget::refreshFromDatabase(
    const std::vector<arcane::application::vo::InventoryItemVO> &items)
{
    m_items = items;
    retranslateUi();
}

QString InventoryWidget::itemId(QTreeWidgetItem *item) const
{
    return item ? item->data(0, Qt::UserRole).toString() : QString();
}
