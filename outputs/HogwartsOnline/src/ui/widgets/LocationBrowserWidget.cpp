#include "ui/widgets/LocationBrowserWidget.h"
#include "core/LanguageManager.h"
#include "core/LocationData.h"
#include "core/DeepSeekClient.h"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

LocationBrowserWidget::LocationBrowserWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("LocationBrowserWidget");
    buildUi();
    retranslateUi();
}

void LocationBrowserWidget::buildUi()
{
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(0);

    // 左侧场景列表
    auto *leftPanel = new QWidget(this);
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(8, 6, 8, 6);
    leftLayout->setSpacing(6);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("PanelTitle");
    m_titleLabel->setMinimumWidth(200);
    leftLayout->addWidget(m_titleLabel);

    m_locationList = new QListWidget(this);
    m_locationList->setObjectName("LocationList");
    m_locationList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_locationList->setMinimumWidth(220);
    m_locationList->setMaximumWidth(320);
    leftLayout->addWidget(m_locationList, 1);

    // 右侧详情面板
    auto *rightPanel = new QWidget(this);
    rightPanel->setObjectName("LocationDetailPanel");
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(18, 14, 18, 14);
    rightLayout->setSpacing(12);

    m_detailTitle = new QLabel(this);
    m_detailTitle->setObjectName("LoginTitle");
    m_detailTitle->setWordWrap(true);
    QFont titleFont("Georgia", 22, QFont::Bold);
    m_detailTitle->setFont(titleFont);

    m_detailOnline = new QLabel(this);
    m_detailOnline->setObjectName("ValueLabel");
    QFont onlineFont("Georgia", 12);
    m_detailOnline->setFont(onlineFont);

    m_detailDesc = new QLabel(this);
    m_detailDesc->setWordWrap(true);
    QFont descFont("Georgia", 13);
    m_detailDesc->setFont(descFont);
    m_detailDesc->setObjectName("LocationDesc");

    m_detailNpcs = new QLabel(this);
    m_detailNpcs->setWordWrap(true);
    QFont npcFont("Georgia", 12);
    m_detailNpcs->setFont(npcFont);

    auto *travelButton = new QPushButton(this);
    travelButton->setObjectName("PrimaryButton");
    QFont btnFont("Georgia", 13, QFont::Bold);
    travelButton->setFont(btnFont);
    travelButton->setMinimumHeight(44);
    travelButton->setText("Travel");

    rightLayout->addWidget(m_detailTitle);
    rightLayout->addWidget(m_detailOnline);
    rightLayout->addWidget(m_detailDesc);
    rightLayout->addWidget(m_detailNpcs);
    rightLayout->addStretch();
    rightLayout->addWidget(travelButton, 0, Qt::AlignLeft);

    rootLayout->addWidget(leftPanel);
    rootLayout->addWidget(rightPanel, 1);

    connect(m_locationList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const QString id = item->data(Qt::UserRole).toString();
        if (id.isEmpty()) return;
        m_currentId = id;

        const auto locations = LocationPresets::all();
        for (const auto &loc : locations) {
            if (loc.id == id) {
                auto &lm = LanguageManager::instance();
                bool isCn = lm.currentLanguage() == Language::Chinese;
                m_detailTitle->setText(QString("%1 %2")
                    .arg(loc.emoji, isCn ? loc.nameCn : loc.nameEn));
                m_detailDesc->setText(isCn ? loc.descriptionCn : loc.descriptionEn);
                m_detailOnline->setText(isCn
                    ? QString("在线人数：%1").arg(loc.onlineCount)
                    : QString("Online: %1").arg(loc.onlineCount));
                break;
            }
        }
    });

    connect(travelButton, &QPushButton::clicked, this, [this]() {
        if (!m_currentId.isEmpty()) {
            emit travelRequested(m_currentId);
        }
    });
}

void LocationBrowserWidget::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    bool isCn = lm.currentLanguage() == Language::Chinese;
    m_titleLabel->setText(isCn ? "🏰 魔法世界" : "🏰 Wizarding World");
    rebuildList();
}

void LocationBrowserWidget::rebuildList()
{
    if (!m_locationList) return;
    m_locationList->clear();

    auto &lm = LanguageManager::instance();
    bool isCn = lm.currentLanguage() == Language::Chinese;
    const auto locations = LocationPresets::all();

    // 按分类分组
    LocationCategory currentCat = LocationCategory::Hogwarts;
    bool firstCategory = true;
    for (int i = 0; i < static_cast<int>(LocationCategory::StMungo) + 1; ++i) {
        const auto cat = static_cast<LocationCategory>(i);

        // 分类头
        auto *header = new QListWidgetItem(m_locationList);
        header->setText(LocationPresets::categoryName(cat, isCn));
        header->setFlags(header->flags() & ~Qt::ItemIsSelectable);
        QFont headerFont("Georgia", 12, QFont::Bold);
        header->setFont(headerFont);
        header->setForeground(QColor("#f3d27a"));
        header->setBackground(QColor("#3a251a"));
        header->setData(Qt::UserRole, QString());
        header->setSizeHint(QSize(0, 34));

        for (const auto &loc : locations) {
            if (loc.category != cat) continue;

            auto *item = new QListWidgetItem(m_locationList);
            item->setText(QString("  %1 %2")
                .arg(loc.emoji, isCn ? loc.nameCn : loc.nameEn));
            item->setData(Qt::UserRole, loc.id);
            item->setToolTip(isCn ? loc.descriptionCn : loc.descriptionEn);
            item->setSizeHint(QSize(0, 36));
        }
    }
}

void LocationBrowserWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
