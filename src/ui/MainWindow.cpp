#include "ui/MainWindow.h"

#include "ui/ApiKeyDialog.h"
#include "ui/I18n.hpp"
#include "ui/widgets/CampusTimeWidget.h"
#include "ui/widgets/CharacterStatusWidget.h"
#include "ui/widgets/ChatEventWidget.h"
#include "ui/widgets/CourseScheduleWidget.h"
#include "ui/widgets/HouseRankingWidget.h"
#include "ui/widgets/InventoryWidget.h"
#include "ui/widgets/LocationPanel.h"

#include <QAction>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setObjectName("MainWindow");
    setWindowTitle(TR("title.app"));
    resize(1440, 900);
    setMinimumSize(1040, 720);

    buildMenu();
    buildUi();
    connectWidgetSignals();

    // 监听语言切换，刷新所有面板。
    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) { retranslateUi(); });

    statusBar()->showMessage(TR("chat.feedback.disconnected"));
}

void MainWindow::setPlayerIdentity(const QString &studentName, const QString &houseName)
{
    m_characterStatus->setIdentity(studentName, houseName);
}

void MainWindow::appendCampusMessage(const QString &channel, const QString &speaker, const QString &text)
{
    m_chatEvents->appendMessage(channel, speaker, text);
}

void MainWindow::updatePlayerLocation(const QString &location, const QString &state)
{
    m_characterStatus->setCampusState(location, state);
    updateCurrentLocationPanel(location);
}

void MainWindow::showFeedback(const QString &text)
{
    statusBar()->showMessage(text, 5000);
}

void MainWindow::displayMaraudersMap(const QString &spellQuote,
                                     std::uint32_t onlineCount,
                                     const std::vector<arcane::application::vo::MapPlayerVO> &players)
{
    appendCampusMessage(QStringLiteral("Map"), QStringLiteral("Marauder's Map"),
                        spellQuote);
    appendCampusMessage(QStringLiteral("Map"), QStringLiteral("System"),
                        QString::number(onlineCount)
                            + (onlineCount == 1 ? " wanderer is currently within the grounds."
                                                : " wanderers are currently within the grounds."));
    for (const auto &player : players) {
        const auto line = QString::fromStdString(player.statusEmoji)
                              + QStringLiteral(" ")
                              + QString::fromStdString(player.nickname)
                              + QStringLiteral(" (")
                              + QString::fromStdString(player.house)
                              + QStringLiteral(") — ")
                              + QString::fromStdString(player.roomName)
                              + QStringLiteral(" @ (")
                              + QString::number(player.posX, 'f', 1)
                              + QStringLiteral(", ")
                              + QString::number(player.posY, 'f', 1)
                              + QStringLiteral(")");
        appendCampusMessage(QStringLiteral("Map"), QStringLiteral("Reveal"), line);
    }
}

void MainWindow::refreshInventory(const std::vector<arcane::application::vo::InventoryItemVO> &items)
{
    m_inventory->refreshFromDatabase(items);
}

void MainWindow::onHousePointsChanged(const QString &house, int delta, const QString &reason)
{
    m_houseRanking->onHousePointsChanged(house, delta, reason);
}

void MainWindow::appendRichCampusMessage(const QString &channel, const QString &speaker, const QString &htmlBody)
{
    m_chatEvents->appendRichMessage(channel, speaker, htmlBody);
}

void MainWindow::setSceneHeader(const QString &title)
{
    m_chatEvents->setSceneHeader(title);
}

void MainWindow::setInputHint(const QString &hint)
{
    m_chatEvents->setInputHint(hint);
}

void MainWindow::presentChoices(const QStringList &labels)
{
    m_chatEvents->presentChoices(labels);
}

void MainWindow::buildMenu()
{
    auto *roleMenu = menuBar()->addMenu(TR("menu.character"));
    roleMenu->addAction(TR("menu.character.profile"));
    roleMenu->addAction(TR("menu.character.relationships"));

    auto *campusMenu = menuBar()->addMenu(TR("menu.campus"));
    auto *mapAction = campusMenu->addAction(TR("menu.campus.map"));
    connect(mapAction, &QAction::triggered, this, [this] { emit useMaraudersMapRequested(0); });
    auto *nightPatrolAction = campusMenu->addAction(TR("menu.campus.nightpatrol"));
    connect(nightPatrolAction, &QAction::triggered, this, [this] { emit nightPatrolRequested({}); });
    campusMenu->addAction(TR("menu.campus.courses"));
    campusMenu->addAction(TR("menu.campus.activities"));

    // 剧情菜单：每个选择都会真实影响风评与人际关系。
    auto *storyMenu = menuBar()->addMenu(TR("menu.story"));
    auto *beginPrologueAction = storyMenu->addAction(QStringLiteral("Begin Prologue"));
    connect(beginPrologueAction, &QAction::triggered, this, [this] { emit startPrologueRequested(); });
    storyMenu->addSeparator();
    auto *reputationAction = storyMenu->addAction(TR("story.viewrep"));
    connect(reputationAction, &QAction::triggered, this, [this] { emit reputationQueryRequested(); });
    storyMenu->addSeparator();
    auto addChoice = [&](const char *id, const char *labelKey) {
        auto *a = storyMenu->addAction(TRQ(labelKey));
        connect(a, &QAction::triggered, this, [this, id] { emit storyChoiceRequested(QString::fromLatin1(id)); });
    };
    addChoice("share_notes",        "story.share");
    addChoice("restricted_section", "story.restricted");
    addChoice("stand_up_for_friend","story.standup");
    addChoice("cheat_potions_exam", "story.cheat");
    addChoice("help_hagrid",       "story.hagrid");
    addChoice("duel_malfoy",        "story.duel");
    addChoice("study_late_library","story.study");

    auto *socialMenu = menuBar()->addMenu(TR("menu.social"));
    socialMenu->addAction("Friends");
    socialMenu->addAction("Owl Mail");

    auto *systemMenu = menuBar()->addMenu(TR("menu.system"));
    auto *refreshInventoryAction = systemMenu->addAction(TR("menu.system.refreshinv"));
    connect(refreshInventoryAction, &QAction::triggered, this, [this] { emit refreshInventoryRequested(); });
    auto *aiSettingsAction = systemMenu->addAction(TR("menu.system.aisettings"));
    connect(aiSettingsAction, &QAction::triggered, this, [this] {
        ApiKeyDialog dialog(this);
        dialog.exec();
    });
    systemMenu->addAction(TR("menu.system.logout"));
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    central->setObjectName("WorldShell");
    central->setContentsMargins(0, 0, 0, 0);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(14, 12, 14, 14);
    rootLayout->setSpacing(12);

    // ===== 世界抬头：抽屉入口 + 世界标题 + 时间 =====
    auto *topBar = new QWidget(central);
    topBar->setObjectName("WorldHeader");
    auto *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(14, 10, 14, 10);
    topBarLayout->setSpacing(12);

    m_leftDrawerButton = new QToolButton(topBar);
    m_leftDrawerButton->setObjectName("DrawerToggle");
    m_leftDrawerButton->setCheckable(true);
    m_leftDrawerButton->setToolButtonStyle(Qt::ToolButtonTextOnly);

    auto *worldHeading = new QWidget(topBar);
    worldHeading->setObjectName("WorldHeading");
    auto *worldHeadingLayout = new QVBoxLayout(worldHeading);
    worldHeadingLayout->setContentsMargins(0, 0, 0, 0);
    worldHeadingLayout->setSpacing(1);

    auto *eyebrow = new QLabel(QStringLiteral("ARCANE CAMPUS ONLINE"), worldHeading);
    eyebrow->setObjectName("WorldEyebrow");
    m_worldTitleLabel = new QLabel(worldHeading);
    m_worldTitleLabel->setObjectName("WorldTitle");
    m_worldSubtitleLabel = new QLabel(worldHeading);
    m_worldSubtitleLabel->setObjectName("WorldSubtitle");
    worldHeadingLayout->addWidget(eyebrow);
    worldHeadingLayout->addWidget(m_worldTitleLabel);
    worldHeadingLayout->addWidget(m_worldSubtitleLabel);

    m_campusTime = new CampusTimeWidget(topBar);
    m_campusTime->setObjectName("HeaderCampusTime");
    m_campusTime->setMaximumWidth(390);

    m_rightDrawerButton = new QToolButton(topBar);
    m_rightDrawerButton->setObjectName("DrawerToggle");
    m_rightDrawerButton->setCheckable(true);
    m_rightDrawerButton->setToolButtonStyle(Qt::ToolButtonTextOnly);

    m_languageCombo = new QComboBox(topBar);
    m_languageCombo->setObjectName("LanguageCombo");
    m_languageCombo->addItem(QStringLiteral("中文"), static_cast<int>(arcane::ui::I18n::Lang::Chinese));
    m_languageCombo->addItem(QStringLiteral("English"), static_cast<int>(arcane::ui::I18n::Lang::English));
    m_languageCombo->setCurrentIndex(static_cast<int>(arcane::ui::I18n::instance().language()));
    m_languageCombo->setMinimumWidth(110);
    connect(m_languageCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                arcane::ui::I18n::instance().setLanguage(
                    static_cast<arcane::ui::I18n::Lang>(m_languageCombo->itemData(index).toInt()));
            });

    topBarLayout->addWidget(m_leftDrawerButton, 0, Qt::AlignVCenter);
    topBarLayout->addWidget(worldHeading, 1);
    topBarLayout->addWidget(m_campusTime, 0, Qt::AlignVCenter);
    topBarLayout->addWidget(m_rightDrawerButton, 0, Qt::AlignVCenter);
    topBarLayout->addWidget(m_languageCombo, 0, Qt::AlignVCenter);

    rootLayout->addWidget(topBar);

    // ===== 主舞台：中央叙事始终优先，左右功能按需唤出 =====
    auto *stageLayout = new QHBoxLayout;
    stageLayout->setContentsMargins(0, 0, 0, 0);
    stageLayout->setSpacing(10);

    m_leftDrawer = new QFrame(central);
    m_leftDrawer->setObjectName("AuxDrawer");
    m_leftDrawer->setFixedWidth(300);
    auto *leftDrawerLayout = new QVBoxLayout(m_leftDrawer);
    leftDrawerLayout->setContentsMargins(8, 8, 8, 8);
    leftDrawerLayout->setSpacing(8);

    auto *leftHeader = new QWidget(m_leftDrawer);
    leftHeader->setObjectName("DrawerHeader");
    auto *leftHeaderLayout = new QHBoxLayout(leftHeader);
    leftHeaderLayout->setContentsMargins(8, 4, 4, 4);
    m_leftDrawerTitle = new QLabel(leftHeader);
    m_leftDrawerTitle->setObjectName("DrawerTitle");
    auto *leftClose = new QToolButton(leftHeader);
    leftClose->setObjectName("DrawerCloseButton");
    leftClose->setText(QStringLiteral("\u00d7"));
    leftClose->setToolTip(TR("drawer.close"));
    leftHeaderLayout->addWidget(m_leftDrawerTitle, 1);
    leftHeaderLayout->addWidget(leftClose);

    m_locationPanel = new LocationPanel(m_leftDrawer);
    m_locationPanel->setMinimumWidth(0);
    m_locationPanel->setMaximumWidth(QWIDGETSIZE_MAX);
    leftDrawerLayout->addWidget(leftHeader);
    leftDrawerLayout->addWidget(m_locationPanel, 1);

    m_chatEvents = new ChatEventWidget(central);
    m_chatEvents->setObjectName("NarrativeStage");

    m_rightDrawer = new QFrame(central);
    m_rightDrawer->setObjectName("AuxDrawer");
    m_rightDrawer->setFixedWidth(380);
    auto *rightDrawerLayout = new QVBoxLayout(m_rightDrawer);
    rightDrawerLayout->setContentsMargins(8, 8, 8, 8);
    rightDrawerLayout->setSpacing(8);

    auto *rightHeader = new QWidget(m_rightDrawer);
    rightHeader->setObjectName("DrawerHeader");
    auto *rightHeaderLayout = new QHBoxLayout(rightHeader);
    rightHeaderLayout->setContentsMargins(8, 4, 4, 4);
    m_rightDrawerTitle = new QLabel(rightHeader);
    m_rightDrawerTitle->setObjectName("DrawerTitle");
    auto *rightClose = new QToolButton(rightHeader);
    rightClose->setObjectName("DrawerCloseButton");
    rightClose->setText(QStringLiteral("\u00d7"));
    rightClose->setToolTip(TR("drawer.close"));
    rightHeaderLayout->addWidget(m_rightDrawerTitle, 1);
    rightHeaderLayout->addWidget(rightClose);

    m_rightTabs = new QTabWidget(m_rightDrawer);
    m_rightTabs->setObjectName("JournalTabs");
    m_rightTabs->setDocumentMode(true);
    m_characterStatus = new CharacterStatusWidget(m_rightTabs);
    m_houseRanking = new HouseRankingWidget(m_rightTabs);
    m_courses = new CourseScheduleWidget(m_rightTabs);
    m_inventory = new InventoryWidget(m_rightTabs);
    m_rightTabs->addTab(m_characterStatus, QString());
    m_rightTabs->addTab(m_houseRanking, QString());
    m_rightTabs->addTab(m_courses, QString());
    m_rightTabs->addTab(m_inventory, QString());
    rightDrawerLayout->addWidget(rightHeader);
    rightDrawerLayout->addWidget(m_rightTabs, 1);

    stageLayout->addWidget(m_leftDrawer);
    stageLayout->addWidget(m_chatEvents, 1);
    stageLayout->addWidget(m_rightDrawer);
    rootLayout->addLayout(stageLayout, 1);

    connect(m_leftDrawerButton, &QToolButton::clicked,
            this, [this] { setLeftDrawerVisible(!m_leftDrawer->isVisible()); });
    connect(m_rightDrawerButton, &QToolButton::clicked,
            this, [this] { setRightDrawerVisible(!m_rightDrawer->isVisible()); });
    connect(leftClose, &QToolButton::clicked,
            this, [this] { setLeftDrawerVisible(false); });
    connect(rightClose, &QToolButton::clicked,
            this, [this] { setRightDrawerVisible(false); });

    m_leftDrawer->hide();
    m_rightDrawer->hide();
    updateDrawerControls();
    retranslateUi();

    setCentralWidget(central);
}

void MainWindow::setLeftDrawerVisible(bool visible)
{
    if (!m_leftDrawer) return;
    if (visible && m_rightDrawer) m_rightDrawer->hide();
    m_leftDrawer->setVisible(visible);
    updateDrawerControls();
}

void MainWindow::setRightDrawerVisible(bool visible)
{
    if (!m_rightDrawer) return;
    if (visible && m_leftDrawer) m_leftDrawer->hide();
    m_rightDrawer->setVisible(visible);
    updateDrawerControls();
}

void MainWindow::updateDrawerControls()
{
    const bool leftVisible = m_leftDrawer && m_leftDrawer->isVisible();
    const bool rightVisible = m_rightDrawer && m_rightDrawer->isVisible();
    if (m_leftDrawerButton) m_leftDrawerButton->setChecked(leftVisible);
    if (m_rightDrawerButton) m_rightDrawerButton->setChecked(rightVisible);
}

void MainWindow::connectWidgetSignals()
{
    connect(m_chatEvents, &ChatEventWidget::sendChatRequested,
            this, &MainWindow::sendChatRequested);
    // 开局剧情选项按钮点击 → 转发到 Controller。
    connect(m_chatEvents, &ChatEventWidget::choiceSelected,
            this, &MainWindow::prologueChoiceSelected);
    // LocationPanel（左栏）发出的所有交互信号统一转发到对应的主信号。
    connect(m_locationPanel, &LocationPanel::moveToLocationRequested,
            this, &MainWindow::moveToLocationRequested);
    connect(m_locationPanel, &LocationPanel::studyRequested,
            this, &MainWindow::studyRequested);
    connect(m_locationPanel, &LocationPanel::joinActivityRequested,
            this, &MainWindow::activityJoinRequested);
    connect(m_locationPanel, &LocationPanel::useMaraudersMapRequested,
            this, [this](std::uint64_t roomId) { emit useMaraudersMapRequested(roomId); });
    connect(m_locationPanel, &LocationPanel::nightPatrolRequested,
            this, &MainWindow::nightPatrolRequested);
    connect(m_locationPanel, &LocationPanel::memberSelected,
            this, &MainWindow::memberSelected);
    // 课程 / 背包 / 时钟
    connect(m_courses, &CourseScheduleWidget::goToClassroomRequested,
            this, &MainWindow::joinClassRequested);
    connect(m_courses, &CourseScheduleWidget::courseDetailRequested,
            this, &MainWindow::courseDetailsRequested);
    connect(m_courses, &CourseScheduleWidget::professorInfoRequested,
            this, &MainWindow::professorInfoRequested);
    connect(m_inventory, &InventoryWidget::itemUseRequested,
            this, &MainWindow::itemUseRequested);
    connect(m_inventory, &InventoryWidget::itemInspectRequested,
            this, &MainWindow::itemInspectRequested);
    connect(m_inventory, &InventoryWidget::itemGiftRequested,
            this, &MainWindow::itemGiftRequested);
    connect(m_campusTime, &CampusTimeWidget::timePeriodChanged,
            this, &MainWindow::timePeriodChanged);
}

void MainWindow::updateCurrentLocationPanel(const QString &location)
{
    if (m_locationPanel) {
        m_locationPanel->setCurrentLocation(location);
    }
}

void MainWindow::retranslateUi()
{
    setWindowTitle(TR("title.app"));
    if (m_worldTitleLabel) m_worldTitleLabel->setText(TR("shell.world.title"));
    if (m_worldSubtitleLabel) m_worldSubtitleLabel->setText(TR("shell.world.subtitle"));
    if (m_leftDrawerTitle) m_leftDrawerTitle->setText(TR("drawer.scene.title"));
    if (m_rightDrawerTitle) m_rightDrawerTitle->setText(TR("drawer.journal.title"));
    if (m_leftDrawerButton) m_leftDrawerButton->setText(TR("drawer.scene.toggle"));
    if (m_rightDrawerButton) m_rightDrawerButton->setText(TR("drawer.journal.toggle"));
    if (m_rightTabs && m_rightTabs->count() == 4) {
        m_rightTabs->setTabText(0, TR("drawer.tab.status"));
        m_rightTabs->setTabText(1, TR("drawer.tab.house"));
        m_rightTabs->setTabText(2, TR("drawer.tab.classes"));
        m_rightTabs->setTabText(3, TR("drawer.tab.inventory"));
    }
    if (statusBar()) {
        statusBar()->showMessage(TR("chat.feedback.disconnected"));
    }
    // 子面板在构造时已各自连接 languageChanged → retranslateUi；
    // 这里重建菜单栏以应用新语言的菜单标签。
    menuBar()->clear();
    buildMenu();
}
