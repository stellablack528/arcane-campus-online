#include "ui/widgets/MemberListWidget.h"

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
}

void MemberListWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *title = new QLabel("Online Campus", this);
    title->setObjectName("PanelTitle");

    m_tree = new QTreeWidget(this);
    m_tree->setColumnCount(4);
    m_tree->setHeaderLabels({"Name", "Place", "State", "House"});
    m_tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tree->header()->setStretchLastSection(false);
    m_tree->setRootIsDecorated(true);
    m_tree->setAlternatingRowColors(true);

    auto *players = addGroup("Players");
    auto *professors = addGroup("Professors");
    auto *studentNpcs = addGroup("Student NPCs");
    auto *ghosts = addGroup("Ghosts");
    auto *shops = addGroup("Shop NPCs");

    addMember(players, "player_001", "Luna Aster", "Great Hall", "Online", "Ravenclaw");
    addMember(players, "player_002", "Cedric Vale", "Courtyard", "Online", "Hufflepuff");
    addMember(professors, "prof_mcgonagall", "Professor McGonagall", "Transfiguration", "Teaching", "Staff");
    addMember(studentNpcs, "npc_hermione", "Hermione Granger", "Library", "Reading", "Gryffindor");
    addMember(studentNpcs, "npc_gareth", "Gareth Weasley", "Great Hall", "Chatting", "Gryffindor");
    addMember(ghosts, "ghost_gray_lady", "The Grey Lady", "Ravenclaw Tower", "Wandering", "Ravenclaw");
    addMember(shops, "shop_ollivander", "Wand Shop Clerk", "Hogsmeade", "Available", "None");

    m_tree->expandAll();

    layout->addWidget(title);
    layout->addWidget(m_tree, 1);

    connect(m_tree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item) {
        const QString id = item->data(0, Qt::UserRole).toString();
        if (!id.isEmpty()) {
            emit memberSelected(id);
        }
    });
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
