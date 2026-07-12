#include "ui/widgets/InventoryWidget.h"

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
}

void InventoryWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *title = new QLabel("Inventory", this);
    title->setObjectName("PanelTitle");

    m_list = new QListWidget(this);
    m_list->setObjectName("InventoryList");
    m_list->setViewMode(QListView::IconMode);
    m_list->setResizeMode(QListView::Adjust);
    m_list->setMovement(QListView::Static);
    m_list->setIconSize(QSize(32, 32));
    m_list->setGridSize(QSize(120, 64));
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);

    addItem("wand", "Wand", "A simple student wand.");
    addItem("chocolate_frog", "Chocolate Frog", "A lively sweet wrapped in blue foil.");
    addItem("butterbeer", "Butterbeer", "A warm drink from Hogsmeade.");
    addItem("potion", "Potion", "A small bottle of shimmering potion.");
    addItem("quill", "Quill", "Useful for class notes and letters.");
    addItem("galleon", "Galleons", "Wizarding currency.");

    layout->addWidget(title);
    layout->addWidget(m_list, 1);

    connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        emit itemUseRequested(itemId(item));
    });

    connect(m_list, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        auto *item = m_list->itemAt(pos);
        if (!item) {
            return;
        }

        QMenu menu(this);
        auto *inspect = menu.addAction("Inspect");
        auto *gift = menu.addAction("Gift to NPC");
        auto *chosen = menu.exec(m_list->mapToGlobal(pos));
        if (chosen == inspect) {
            emit itemInspectRequested(itemId(item));
        } else if (chosen == gift) {
            emit itemGiftRequested(itemId(item), "npc_hermione");
        }
    });
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
