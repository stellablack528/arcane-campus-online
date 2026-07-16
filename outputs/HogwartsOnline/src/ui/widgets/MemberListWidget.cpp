#include "ui/widgets/MemberListWidget.h"
#include "core/LanguageManager.h"

#include <QEvent>
#include <QHeaderView>
#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

MemberListWidget::MemberListWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("MemberListWidget");
    buildUi();
    retranslateUi();
}

void MemberListWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("PanelTitle");

    m_tree = new QTreeWidget(this);
    m_tree->setColumnCount(4);
    m_tree->setRootIsDecorated(true);
    m_tree->setAlternatingRowColors(true);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_tree, 1);

    connect(m_tree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item) {
        const QString id = item->data(0, Qt::UserRole).toString();
        if (!id.isEmpty()) {
            emit memberSelected(id);
        }
    });
}

void MemberListWidget::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    m_titleLabel->setText(isCn ? "校园在线成员" : "Online Campus");

    m_tree->setHeaderLabels({
        isCn ? "姓名" : "Name",
        isCn ? "位置" : "Place",
        isCn ? "状态" : "State",
        isCn ? "学院" : "House"
    });
    m_tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tree->header()->setStretchLastSection(false);
    refreshTree();
}

void MemberListWidget::refreshTree()
{
    if (!m_tree) return;
    m_tree->clear();
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    QTreeWidgetItem *players, *professors, *studentNpcs, *ghosts, *shops;
    if (isCn) {
        players = addGroup("玩家 (Players)");
        professors = addGroup("教授 (Professors)");
        studentNpcs = addGroup("学生 NPC (Student NPCs)");
        ghosts = addGroup("幽灵 (Ghosts)");
        shops = addGroup("商店 NPC (Shop NPCs)");

        addMember(players, "player_001", "Luna Aster", "礼堂", "在线", "拉文克劳");
        addMember(players, "player_002", "Cedric Vale", "庭院", "在线", "赫奇帕奇");
        addMember(professors, "prof_mcgonagall", "麦格教授", "变形术教室", "授课中", "教职");
        addMember(studentNpcs, "npc_hermione", "赫敏·格兰杰", "图书馆", "阅读中", "格兰芬多");
        addMember(studentNpcs, "npc_gareth", "加雷斯·韦斯莱", "礼堂", "聊天中", "格兰芬多");
        addMember(ghosts, "ghost_gray_lady", "灰女士", "拉文克劳塔", "游荡中", "拉文克劳");
        addMember(shops, "shop_ollivander", "魔杖店店员", "霍格莫德", "营业中", "无");
    } else {
        players = addGroup("Players");
        professors = addGroup("Professors");
        studentNpcs = addGroup("Student NPCs");
        ghosts = addGroup("Ghosts");
        shops = addGroup("Shop NPCs");

        addMember(players, "player_001", "Luna Aster", "Great Hall", "Online", "Ravenclaw");
        addMember(players, "player_002", "Cedric Vale", "Courtyard", "Online", "Hufflepuff");
        addMember(professors, "prof_mcgonagall", "Professor McGonagall", "Transfiguration", "Teaching", "Staff");
        addMember(studentNpcs, "npc_hermione", "Hermione Granger", "Library", "Reading", "Gryffindor");
        addMember(studentNpcs, "npc_gareth", "Gareth Weasley", "Great Hall", "Chatting", "Gryffindor");
        addMember(ghosts, "ghost_gray_lady", "The Grey Lady", "Ravenclaw Tower", "Wandering", "Ravenclaw");
        addMember(shops, "shop_ollivander", "Wand Shop Clerk", "Hogsmeade", "Available", "None");
    }

    m_tree->expandAll();
}

QTreeWidgetItem *MemberListWidget::addGroup(const QString &title)
{
    auto *item = new QTreeWidgetItem(m_tree);
    item->setText(0, title);
    item->setFirstColumnSpanned(true);
    return item;
}

void MemberListWidget::addMember(QTreeWidgetItem *group,
                                 const QString &id,
                                 const QString &name,
                                 const QString &location,
                                 const QString &status,
                                 const QString &house)
{
    auto *item = new QTreeWidgetItem(group);
    item->setText(0, name);
    item->setText(1, location);
    item->setText(2, status);
    item->setText(3, house);
    item->setData(0, Qt::UserRole, id);
}

void MemberListWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
