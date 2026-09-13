#include "ui/MainWindow.h"

#include "ui/ApiKeyDialog.h"
#include "ui/I18n.hpp"
#include "ui/widgets/CampusTimeWidget.h"
#include "ui/widgets/CharacterStatusWidget.h"
#include "ui/widgets/ChatEventWidget.h"
#include "ui/widgets/CourseScheduleWidget.h"
#include "ui/widgets/HouseRankingWidget.h"
#include "ui/widgets/InventoryWidget.h"
#include "ui/widgets/LocationActionBar.h"
#include "ui/widgets/LocationPanel.h"
#include "ui/widgets/MemberListWidget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QAction>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setObjectName("MainWindow");
    setWindowTitle(TR("title.app"));
    resize(1440, 900);
    setMinimumSize(1180, 760);

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
    central->setContentsMargins(0, 0, 0, 0);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(8);

    // ===== 顶部条：标题 + 时钟 + 语言切换 =====
    auto *topBar = new QWidget(central);
    auto *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(10);

    m_campusTime = new CampusTimeWidget(topBar);

    // 语言切换 ComboBox（右对齐）
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

    topBarLayout->addWidget(m_campusTime, 1);
    topBarLayout->addWidget(m_languageCombo, 0, Qt::AlignRight | Qt::AlignVCenter);

    rootLayout->addWidget(topBar);

    // ===== 主体三栏：左（地点动作）| 中（聊天）| 右（信息+沙漏+课程+背包） =====
    auto *mainSplitter = new QSplitter(Qt::Horizontal, central);
    mainSplitter->setObjectName("MainSplitter");

    // 左侧：地点面板
    m_locationPanel = new LocationPanel(mainSplitter);
    mainSplitter->addWidget(m_locationPanel);

    // 中间：聊天与事件
    m_chatEvents = new ChatEventWidget(mainSplitter);
    mainSplitter->addWidget(m_chatEvents);

    // 右侧：信息概述（顶）+ 学院沙漏 + 课程 + 背包
    auto *rightColumn = new QWidget(mainSplitter);
    auto *rightLayout = new QVBoxLayout(rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    m_characterStatus = new CharacterStatusWidget(rightColumn);   // 右上：信息概述
    m_houseRanking = new HouseRankingWidget(rightColumn);         // 右中：学院沙漏
    m_courses = new CourseScheduleWidget(rightColumn);            // 右下：今日课程
    m_inventory = new InventoryWidget(rightColumn);               // 右下：背包

    rightLayout->addWidget(m_characterStatus, 4);
    rightLayout->addWidget(m_houseRanking, 3);
    rightLayout->addWidget(m_courses, 2);
    rightLayout->addWidget(m_inventory, 2);

    mainSplitter->addWidget(rightColumn);
    mainSplitter->setSizes({300, 760, 360});
    mainSplitter->setStretchFactor(1, 1);

    rootLayout->addWidget(mainSplitter, 1);

    // 不再使用顶部 LocationActionBar（已合并到 LocationPanel 左栏）。
    // 但保留成员指针以兼容旧信号连接（通过 connectWidgetSignals 转发到 LocationPanel）。
    m_actionBar = new LocationActionBar(this);  // 隐藏备用
    m_actionBar->hide();

    setCentralWidget(central);
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
    if (statusBar()) {
        statusBar()->showMessage(TR("chat.feedback.disconnected"));
    }
    // 子面板在构造时已各自连接 languageChanged → retranslateUi；
    // 这里重建菜单栏以应用新语言的菜单标签。
    menuBar()->clear();
    buildMenu();
}
