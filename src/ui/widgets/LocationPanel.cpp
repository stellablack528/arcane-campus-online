#include "ui/widgets/LocationPanel.h"

#include "ui/I18n.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>

#include <array>

namespace {

struct MoveTarget {
    const char* locationId;
    const char* labelKey;
};

constexpr std::array<MoveTarget, 5> kMoveTargets = {{
    {"great_hall",              "loc.greathall"},
    {"transfiguration_classroom","loc.classroom"},
    {"library",                 "loc.library"},
    {"courtyard",               "loc.courtyard"},
    {"hogsmeade",               "loc.hogsmeade"},
}};

// 此地角色：locationId -> [(memberId, npcNameKey)]
struct Person {
    const char* memberId;
    const char* nameKey;
};

const QHash<QString, std::vector<Person>>& peopleByLocation()
{
    static const QHash<QString, std::vector<Person>> map = {
        {"great_hall", {
            {"npc_gareth", "npc.gareth"},
        }},
        {"library", {
            {"npc_hermione", "npc.hermione"},
        }},
        {"transfiguration_classroom", {
            {"prof_mcgonagall", "npc.mcgonagall"},
        }},
        {"courtyard", {
            {"player_002", "npc.cedric"},
        }},
        {"ravenclaw_tower", {
            {"ghost_gray_lady", "npc.grey"},
        }},
        {"hogsmeade", {
            {"shop_ollivander", "npc.ollivander"},
        }},
    };
    return map;
}

} // namespace

LocationPanel::LocationPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("LocationPanel");
    setMinimumWidth(260);
    setMaximumWidth(320);
    buildUi();
    retranslateUi();
    setCurrentLocation(QStringLiteral("great_hall"));

    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) { retranslateUi(); refreshPeopleHere(); });
}

void LocationPanel::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(12);

    // ---- 当前地点 ----
    auto *locBox = new QGroupBox(this);
    auto *locLayout = new QVBoxLayout(locBox);
    locLayout->setContentsMargins(12, 18, 12, 12);
    locLayout->setSpacing(4);

    m_sectionLocation = new QLabel(locBox);
    m_sectionLocation->setObjectName("SectionCaption");

    m_currentLocationName = new QLabel(locBox);
    m_currentLocationName->setObjectName("CurrentLocationTitle");
    m_currentLocationName->setWordWrap(true);

    locLayout->addWidget(m_sectionLocation);
    locLayout->addWidget(m_currentLocationName);
    root->addWidget(locBox);

    // ---- 快捷动作 ----
    auto *actionBox = new QGroupBox(this);
    auto *actionLayout = new QVBoxLayout(actionBox);
    actionLayout->setContentsMargins(12, 18, 12, 12);
    actionLayout->setSpacing(8);

    m_sectionActions = new QLabel(actionBox);
    m_sectionActions->setObjectName("SectionCaption");

    auto *row1 = new QHBoxLayout;
    m_talkBtn = new QPushButton(actionBox);
    m_talkBtn->setObjectName("LocationActionBtn");
    m_studyBtn = new QPushButton(actionBox);
    m_studyBtn->setObjectName("LocationActionBtn");
    row1->addWidget(m_talkBtn);
    row1->addWidget(m_studyBtn);

    auto *row2 = new QHBoxLayout;
    m_restBtn = new QPushButton(actionBox);
    m_restBtn->setObjectName("LocationActionBtn");
    m_mapBtn = new QPushButton(actionBox);
    m_mapBtn->setObjectName("LocationActionBtn");
    row2->addWidget(m_restBtn);
    row2->addWidget(m_mapBtn);

    auto *row3 = new QHBoxLayout;
    m_nightBtn = new QPushButton(actionBox);
    m_nightBtn->setObjectName("LocationActionBtn");
    row3->addWidget(m_nightBtn);

    actionLayout->addWidget(m_sectionActions);
    actionLayout->addLayout(row1);
    actionLayout->addLayout(row2);
    actionLayout->addLayout(row3);
    root->addWidget(actionBox);

    // ---- 前往 ----
    auto *moveBox = new QGroupBox(this);
    auto *moveLayout = new QVBoxLayout(moveBox);
    moveLayout->setContentsMargins(12, 18, 12, 12);
    moveLayout->setSpacing(8);

    m_sectionMove = new QLabel(moveBox);
    m_sectionMove->setObjectName("SectionCaption");

    for (const auto& target : kMoveTargets) {
        auto *btn = new QPushButton(moveBox);
        btn->setObjectName("LocationMoveBtn");
        moveLayout->addWidget(btn);
        const QString locId = QString::fromLatin1(target.locationId);
        m_moveButtons[locId] = btn;
        connect(btn, &QPushButton::clicked, this, [this, locId] {
            emit moveToLocationRequested(locId);
            if (locId == QLatin1String("library")) {
                emit studyRequested(locId);
            } else if (locId == QLatin1String("hogsmeade")) {
                emit joinActivityRequested(QStringLiteral("hogsmeade_weekend"));
            }
        });
    }
    root->addWidget(moveBox);

    // ---- 此地角色 ----
    auto *hereBox = new QGroupBox(this);
    auto *hereLayout = new QVBoxLayout(hereBox);
    hereLayout->setContentsMargins(12, 18, 12, 12);
    hereLayout->setSpacing(6);

    m_sectionHere = new QLabel(hereBox);
    m_sectionHere->setObjectName("SectionCaption");

    m_peopleList = new QListWidget(hereBox);
    m_peopleList->setObjectName("PeopleHereList");
    m_peopleList->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_peopleList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const QString id = item ? item->data(Qt::UserRole).toString() : QString();
        if (!id.isEmpty()) {
            emit memberSelected(id);
        }
    });

    hereLayout->addWidget(m_sectionHere);
    hereLayout->addWidget(m_peopleList, 1);
    root->addWidget(hereBox, 1);
}

void LocationPanel::retranslateUi()
{
    m_sectionLocation->setText(TR("title.location"));
    m_sectionActions->setText(TR("title.actions"));
    m_sectionMove->setText(TR("title.move"));
    m_sectionHere->setText(TR("title.here"));

    m_talkBtn->setText(TR("action.talk"));
    m_studyBtn->setText(TR("action.study"));
    m_restBtn->setText(TR("action.rest"));
    m_mapBtn->setText(TR("action.marauders_map"));
    m_nightBtn->setText(TR("action.night_patrol"));

    for (const auto& target : kMoveTargets) {
        if (auto *btn = m_moveButtons.value(QString::fromLatin1(target.locationId), nullptr)) {
            btn->setText(TRQ(QString::fromLatin1(target.labelKey)));
        }
    }

    // 更新当前地点名显示。
    auto it = std::find_if(kMoveTargets.begin(), kMoveTargets.end(),
        [this](const MoveTarget& t) { return QString::fromLatin1(t.locationId) == m_currentLocationId; });
    if (it != kMoveTargets.end()) {
        m_currentLocationName->setText(TRQ(QString::fromLatin1(it->labelKey)));
    }
}

void LocationPanel::setCurrentLocation(const QString &locationId)
{
    m_currentLocationId = locationId;

    auto it = std::find_if(kMoveTargets.begin(), kMoveTargets.end(),
        [this](const MoveTarget& t) { return QString::fromLatin1(t.locationId) == m_currentLocationId; });
    if (it != kMoveTargets.end()) {
        m_currentLocationName->setText(TRQ(QString::fromLatin1(it->labelKey)));
    } else {
        m_currentLocationName->setText(locationId);
    }

    // 图书馆以外隐藏"学习"按钮（避免误点）。
    const bool atLibrary = (locationId == QLatin1String("library"));
    m_studyBtn->setVisible(atLibrary);

    refreshPeopleHere();
}

void LocationPanel::refreshPeopleHere()
{
    m_peopleList->clear();
    const auto& map = peopleByLocation();
    const auto it = map.constFind(m_currentLocationId);
    if (it == map.constEnd()) return;
    for (const auto& p : *it) {
        auto *item = new QListWidgetItem(TRQ(QString::fromLatin1(p.nameKey)), m_peopleList);
        item->setData(Qt::UserRole, QString::fromLatin1(p.memberId));
    }
}
