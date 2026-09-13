#include "ui/widgets/InventoryWidget.h"

#include "ui/I18n.hpp"

#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QMenu>
#include <QPoint>
#include <QSize>
#include <QVBoxLayout>

InventoryWidget::InventoryWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("InventoryWidget");
    buildUi();

    m_seed = {
        {"wand",         "item.wand",       "A simple student wand."},
        {"chocolate_frog","item.chocolate", "A lively sweet wrapped in blue foil."},
        {"butterbeer",   "item.butterbeer", "A warm drink from Hogsmeade."},
        {"potion",       "item.potion",     "A small bottle of shimmering potion."},
        {"quill",        "item.quill",      "Useful for class notes and letters."},
        {"galleon",      "item.galleon",    "Wizarding currency."},
    };

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

    m_list = new QListWidget(this);
    m_list->setObjectName("InventoryList");
    m_list->setViewMode(QListView::IconMode);
    m_list->setResizeMode(QListView::Adjust);
    m_list->setMovement(QListView::Static);
    m_list->setIconSize(QSize(32, 32));
    m_list->setGridSize(QSize(120, 64));
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_list, 1);

    connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        emit itemUseRequested(itemId(item));
    });
    connect(m_list, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        auto *item = m_list->itemAt(pos);
        if (!item) return;
        QMenu menu(this);
        auto *inspect = menu.addAction(TR("item.inspect"));
        auto *gift = menu.addAction(TR("item.gift"));
        auto *chosen = menu.exec(m_list->mapToGlobal(pos));
        if (chosen == inspect) {
            emit itemInspectRequested(itemId(item));
        } else if (chosen == gift) {
            emit itemGiftRequested(itemId(item), QStringLiteral("npc_hermione"));
        }
    });
}

void InventoryWidget::retranslateUi()
{
    if (m_titleLabel) m_titleLabel->setText(TR("title.inventory"));

    m_list->clear();
    for (const auto& s : m_seed) {
        addItem(s.id, TRQ(s.nameKey), s.description);
    }
}

void InventoryWidget::refreshFromDatabase(const std::vector<arcane::application::vo::InventoryItemVO> &items)
{
    m_list->clear();
    for (const auto &item : items) {
        const auto id = QString::number(item.itemId);
        const auto tooltip = QString::fromStdString(item.description)
                                 .append(QStringLiteral("\nType: "))
                                 .append(QString::fromStdString(item.itemType))
                                 .append(QStringLiteral("\nQuantity: "))
                                 .append(QString::number(item.quantity));
        addItem(id, QString::fromStdString(item.itemName), tooltip);
    }
}

void InventoryWidget::addItem(const QString &id, const QString &name, const QString &description)
{
    auto *item = new QListWidgetItem(name, m_list);
    item->setData(Qt::UserRole, id);
    item->setToolTip(description);
}

QString InventoryWidget::itemId(QListWidgetItem *item) const
{
    return item ? item->data(Qt::UserRole).toString() : QString();
}