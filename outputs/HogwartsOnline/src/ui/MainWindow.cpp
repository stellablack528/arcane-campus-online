#include "ui/MainWindow.h"
#include "core/LanguageManager.h"
#include "core/DeepSeekClient.h"

#include "ui/widgets/CampusTimeWidget.h"
#include "ui/widgets/CharacterStatusWidget.h"
#include "ui/widgets/ChatEventWidget.h"
#include "ui/widgets/CourseScheduleWidget.h"
#include "ui/widgets/HouseRankingWidget.h"
#include "ui/widgets/InventoryWidget.h"
#include "ui/widgets/LocationBrowserWidget.h"
#include "ui/widgets/MemberListWidget.h"
#include "ui/widgets/NpcChatDialog.h"

#include <QAction>
#include <QActionGroup>
#include <QEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setObjectName("MainWindow");
    resize(1440, 900);
    setMinimumSize(1180, 760);

    // DeepSeek 客户端
    m_deepseek = new DeepSeekClient(this);
    connect(m_deepseek, &DeepSeekClient::replyReceived, this,
            [](const QString &, const QString &reply, bool ok) {
        Q_UNUSED(reply); Q_UNUSED(ok); // 对话弹窗自己处理
    });

    buildMenu();
    buildUi();
    connectWidgetSignals();
    retranslateUi();

    // 监听语言变化
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged,
            this, [this](Language) { retranslateUi(); });
}

void MainWindow::setPlayerIdentity(const QString &studentName, const QString &houseName)
{
    m_characterStatus->setIdentity(studentName, houseName);
}

void MainWindow::setWandInfo(const QString &wood, const QString &core,
                             int lengthInches, const QString &flexibility)
{
    m_characterStatus->setWandInfo(wood, core, lengthInches, flexibility);
}

void MainWindow::buildMenu()
{
    m_characterMenu = menuBar()->addMenu("Character");
    m_campusMenu = menuBar()->addMenu("Campus");
    m_socialMenu = menuBar()->addMenu("Social");
    m_systemMenu = menuBar()->addMenu("System");

    m_characterMenu->addAction("Profile");
    m_characterMenu->addAction("Relationships");
    m_campusMenu->addAction("Map");
    m_campusMenu->addAction("Courses");
    m_campusMenu->addAction("Activities");
    m_socialMenu->addAction("Friends");
    m_socialMenu->addAction("Owl Mail");
    m_languageMenu = m_systemMenu->addMenu("Language");
    m_langGroup = new QActionGroup(this);
    m_langGroup->setExclusive(true);

    auto *enAction = m_languageMenu->addAction("English");
    enAction->setCheckable(true);
    enAction->setData(static_cast<int>(Language::English));
    m_langGroup->addAction(enAction);

    auto *cnAction = m_languageMenu->addAction("中文");
    cnAction->setCheckable(true);
    cnAction->setData(static_cast<int>(Language::Chinese));
    m_langGroup->addAction(cnAction);

    connect(m_langGroup, &QActionGroup::triggered, this, [this](QAction *act) {
        Language lang = static_cast<Language>(act->data().toInt());
        LanguageManager::instance().setLanguage(lang);
    });

    m_systemMenu->addSeparator();
    auto *settingsAction = m_systemMenu->addAction("Settings");
    settingsAction->setObjectName("MenuSettings");
    auto *logoutAction = m_systemMenu->addAction("Logout");
    logoutAction->setObjectName("MenuLogout");
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(10);

    m_campusTime = new CampusTimeWidget(central);
    m_locationBrowser = new LocationBrowserWidget(central);

    auto *topLayout = new QHBoxLayout;
    topLayout->setSpacing(10);
    topLayout->addWidget(m_campusTime, 1);
    topLayout->addWidget(m_locationBrowser, 2);

    auto *mainSplitter = new QSplitter(Qt::Horizontal, central);
    mainSplitter->setObjectName("MainSplitter");

    m_memberList = new MemberListWidget(mainSplitter);
    m_chatEvents = new ChatEventWidget(mainSplitter);

    auto *rightColumn = new QWidget(mainSplitter);
    auto *rightLayout = new QVBoxLayout(rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    m_characterStatus = new CharacterStatusWidget(rightColumn);
    m_houseRanking = new HouseRankingWidget(rightColumn);
    rightLayout->addWidget(m_characterStatus, 3);
    rightLayout->addWidget(m_houseRanking, 2);

    mainSplitter->addWidget(m_memberList);
    mainSplitter->addWidget(m_chatEvents);
    mainSplitter->addWidget(rightColumn);
    mainSplitter->setSizes({260, 820, 300});
    mainSplitter->setStretchFactor(1, 1);

    auto *bottomSplitter = new QSplitter(Qt::Horizontal, central);
    bottomSplitter->setObjectName("BottomSplitter");

    m_courses = new CourseScheduleWidget(bottomSplitter);
    m_inventory = new InventoryWidget(bottomSplitter);
    bottomSplitter->addWidget(m_courses);
    bottomSplitter->addWidget(m_inventory);
    bottomSplitter->setSizes({620, 760});

    rootLayout->addLayout(topLayout);
    rootLayout->addWidget(mainSplitter, 1);
    rootLayout->addWidget(bottomSplitter);

    setCentralWidget(central);
}

void MainWindow::connectWidgetSignals()
{
    connect(m_chatEvents, &ChatEventWidget::sendChatRequested,
            this, &MainWindow::sendChatRequested);
    connect(m_locationBrowser, &LocationBrowserWidget::travelRequested,
            this, &MainWindow::moveToLocationRequested);
    connect(m_courses, &CourseScheduleWidget::goToClassroomRequested,
            this, &MainWindow::joinClassRequested);
    connect(m_inventory, &InventoryWidget::itemUseRequested,
            this, &MainWindow::itemUseRequested);
    connect(m_inventory, &InventoryWidget::itemInspectRequested,
            this, &MainWindow::itemInspectRequested);
    connect(m_inventory, &InventoryWidget::itemGiftRequested,
            this, &MainWindow::itemGiftRequested);
    connect(m_memberList, &MemberListWidget::npcChatRequested,
            this, &MainWindow::onNpcChatRequested);
}

void MainWindow::onNpcChatRequested(const QString &npcId)
{
    const auto npcs = DeepSeekClient::presetNpcs();
    for (const auto &p : npcs) {
        if (p.npcId == npcId) {
            auto *dialog = new NpcChatDialog(m_deepseek, p, this);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            connect(dialog, &NpcChatDialog::dialogClosed, this, [](const QString &) {});
            dialog->show();
            return;
        }
    }
}

void MainWindow::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    setWindowTitle(isCn ? "霍格沃茨在线校园" : "Hogwarts Online");
    statusBar()->showMessage(isCn
        ? "未连接 · 网络层预留 (TCP 实现中)"
        : "Disconnected · Network layer reserved (TCP pending)");

    m_characterMenu->setTitle(isCn ? "角色" : "Character");
    m_campusMenu->setTitle(isCn ? "校园" : "Campus");
    m_socialMenu->setTitle(isCn ? "社交" : "Social");
    m_systemMenu->setTitle(isCn ? "系统" : "System");
    m_languageMenu->setTitle(isCn ? "语言" : "Language");

    // 菜单项重翻译
    auto updateMenu = [](QMenu *menu, const QStringList &items) {
        QList<QAction*> actions = menu->actions();
        int i = 0;
        for (QAction *a : actions) {
            if (a->menu() || a->isSeparator()) continue;
            if (i < items.size()) a->setText(items[i++]);
        }
    };
    updateMenu(m_characterMenu, isCn ? QStringList{"档案", "关系"}
                                     : QStringList{"Profile", "Relationships"});
    updateMenu(m_campusMenu,     isCn ? QStringList{"地图", "课程", "活动"}
                                     : QStringList{"Map", "Courses", "Activities"});
    updateMenu(m_socialMenu,     isCn ? QStringList{"好友", "猫头鹰邮件"}
                                     : QStringList{"Friends", "Owl Mail"});

    // systemMenu 跳过 Language 子菜单和分隔符：剩余 Settings / Logout
    QList<QAction*> sysActions = m_systemMenu->actions();
    for (QAction *a : sysActions) {
        if (a->objectName() == "MenuSettings")
            a->setText(isCn ? "设置" : "Settings");
        else if (a->objectName() == "MenuLogout")
            a->setText(isCn ? "退出登录" : "Logout");
    }

    // 同步语言菜单勾选状态
    for (QAction *a : m_langGroup->actions()) {
        bool isTarget = static_cast<Language>(a->data().toInt()) == lm.currentLanguage();
        a->setChecked(isTarget);
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QMainWindow::changeEvent(event);
}
