#include "ui/widgets/MemberListWidget.h"

#include "ui/I18n.hpp"

#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPoint>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

MemberListWidget::MemberListWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("MemberListWidget");
    buildUi();

    m_seed = {
        {"members.players",    "player_001",         "npc.luna",        "loc.greathall", "npc.status.online",    "house.ravenclaw"},
        {"members.players",    "player_002",         "npc.cedric",      "loc.courtyard", "npc.status.online",    "house.hufflepuff"},
        {"members.professors", "prof_mcgonagall",    "npc.mcgonagall",  "loc.classroom", "npc.status.teaching",  "npc.role.staff"},
        {"members.studentnpcs","npc_hermione",       "npc.hermione",    "loc.library",   "npc.status.reading",   "house.gryffindor"},
        {"members.studentnpcs","npc_gareth",         "npc.gareth",      "loc.greathall", "npc.status.chatting",  "house.gryffindor"},
        {"members.ghosts",     "ghost_gray_lady",    "npc.grey",        "loc.ravenclaw_tower","npc.status.wandering","house.ravenclaw"},
        {"members.shops",      "shop_ollivander",    "npc.ollivander",  "loc.hogsmeade", "npc.status.available", "npc.role.none"},
    };

    retranslateUi();

    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) { retranslateUi(); });
}

void MemberListWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("PanelTitle");

    m_tree = new QTreeWidget(this);
    m_tree->setColumnCount(4);
    m_tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tree->header()->setStretchLastSection(false);
    m_tree->setRootIsDecorated(true);
    m_tree->setAlternatingRowColors(true);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_tree, 1);

    connect(m_tree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item) {
        const QString id = item->data(0, Qt::UserRole).toString();
        if (!id.isEmpty()) {
            emit memberSelected(id);
        }
    });
    connect(m_tree, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        auto* item = m_tree->itemAt(position);
        const QString id = item ? item->data(0, Qt::UserRole).toString() : QString{};
        if (id.isEmpty()) {
            return;
        }
        QMenu menu(this);
        auto* privateChat = menu.addAction(TR("members.privatechat"));
        auto* profile = menu.addAction(TR("members.profile"));
        const auto* selected = menu.exec(m_tree->viewport()->mapToGlobal(position));
        if (selected == privateChat) {
            emit privateChatRequested(id);
        } else if (selected == profile) {
            emit viewProfileRequested(id);
        }
    });
}

void MemberListWidget::retranslateUi()
{
    if (m_titleLabel) m_titleLabel->setText(TR("title.members"));

    QStringList headers;
    headers << TR("members.col.name")
            << TR("members.col.place")
            << TR("members.col.state")
            << TR("members.col.house");
    m_tree->setHeaderLabels(headers);

    m_tree->clear();

    QHash<QString, QTreeWidgetItem*> groups;
    for (const auto& s : m_seed) {
        auto git = groups.constFind(s.groupKey);
        if (git == groups.constEnd()) {
            auto *g = new QTreeWidgetItem(m_tree);
            g->setText(0, TRQ(s.groupKey));
            g->setFirstColumnSpanned(true);
            groups.insert(s.groupKey, g);
            git = groups.constFind(s.groupKey);
        }
        auto *item = new QTreeWidgetItem(git.value());
        item->setText(0, TRQ(s.nameKey));
        item->setText(1, TRQ(s.locationKey));
        item->setText(2, TRQ(s.statusKey));
        item->setText(3, TRQ(s.houseKey));
        item->setData(0, Qt::UserRole, s.id);
    }
    m_tree->expandAll();
}

QTreeWidgetItem *MemberListWidget::addGroup(const QString &titleKey)
{
    auto *item = new QTreeWidgetItem(m_tree);
    item->setText(0, TRQ(titleKey));
    item->setFirstColumnSpanned(true);
    return item;
}

void MemberListWidget::addMember(QTreeWidgetItem *group,
                                 const QString &id,
                                 const QString &nameKey,
                                 const QString &locationKey,
                                 const QString &statusKey,
                                 const QString &houseKey)
{
    auto *item = new QTreeWidgetItem(group);
    item->setText(0, TRQ(nameKey));
    item->setText(1, TRQ(locationKey));
    item->setText(2, TRQ(statusKey));
    item->setText(3, TRQ(houseKey));
    item->setData(0, Qt::UserRole, id);
}