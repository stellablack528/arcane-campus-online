#include "ui/widgets/InventoryWidget.h"
#include "core/LanguageManager.h"

#include <QEvent>
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
    retranslateUi();
}

void InventoryWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

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
        if (!item) {
            return;
        }

        auto &lm = LanguageManager::instance();
        const bool isCn = (lm.currentLanguage() == Language::Chinese);
        QMenu menu(this);
        auto *inspect = menu.addAction(isCn ? "🔍 查看" : "🔍 Inspect");
        auto *gift = menu.addAction(isCn ? "🎁 赠予 NPC" : "🎁 Gift to NPC");
        auto *chosen = menu.exec(m_list->mapToGlobal(pos));
        if (chosen == inspect) {
            emit itemInspectRequested(itemId(item));
        } else if (chosen == gift) {
            emit itemGiftRequested(itemId(item), "npc_hermione");
        }
    });
}

void InventoryWidget::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);
    m_titleLabel->setText(isCn ? "🎒 背包" : "🎒 Inventory");
    refreshItems();
}

void InventoryWidget::refreshItems()
{
    if (!m_list) return;
    m_list->clear();
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    if (isCn) {
        addItem("wand", "🪄 魔杖", "你的专属魔杖，陪伴你每一段魔法旅程。");
        addItem("chocolate_frog", "🍫 巧克力蛙", "蓝色锡纸包装的活巧克力，会跳来跳去。");
        addItem("butterbeer", "🍺 黄油啤酒", "来自霍格莫德村的暖心饮料。");
        addItem("potion", "🧪 魔药", "一小瓶闪闪发光的药剂。");
        addItem("quill", "🖋️ 羽毛笔", "适合记课堂笔记和写信。");
        addItem("galleon", "🪙 金加隆", "巫师世界的货币。");
    } else {
        addItem("wand", "🪄 Wand", "A simple student wand.");
        addItem("chocolate_frog", "🍫 Chocolate Frog", "A lively sweet wrapped in blue foil.");
        addItem("butterbeer", "🍺 Butterbeer", "A warm drink from Hogsmeade.");
        addItem("potion", "🧪 Potion", "A small bottle of shimmering potion.");
        addItem("quill", "🖋️ Quill", "Useful for class notes and letters.");
        addItem("galleon", "🪙 Galleons", "Wizarding currency.");
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

void InventoryWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
