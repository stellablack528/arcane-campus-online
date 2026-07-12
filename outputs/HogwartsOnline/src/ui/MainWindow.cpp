#include "ui/MainWindow.h"

#include "ui/widgets/CampusTimeWidget.h"
#include "ui/widgets/CharacterStatusWidget.h"
#include "ui/widgets/ChatEventWidget.h"
#include "ui/widgets/CourseScheduleWidget.h"
#include "ui/widgets/HouseRankingWidget.h"
#include "ui/widgets/InventoryWidget.h"
#include "ui/widgets/LocationActionBar.h"
#include "ui/widgets/MemberListWidget.h"

#include <QHBoxLayout>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setObjectName("MainWindow");
    setWindowTitle("Hogwarts Online");
    resize(1440, 900);
    setMinimumSize(1180, 760);

    buildMenu();
    buildUi();
    connectWidgetSignals();

    statusBar()->showMessage("Disconnected · Network layer reserved");
}

void MainWindow::setPlayerIdentity(const QString &studentName, const QString &houseName)
{
    m_characterStatus->setIdentity(studentName, houseName);
}

void MainWindow::buildMenu()
{
    auto *roleMenu = menuBar()->addMenu("Character");
    roleMenu->addAction("Profile");
    roleMenu->addAction("Relationships");

    auto *campusMenu = menuBar()->addMenu("Campus");
    campusMenu->addAction("Map");
    campusMenu->addAction("Courses");
    campusMenu->addAction("Activities");

    auto *socialMenu = menuBar()->addMenu("Social");
    socialMenu->addAction("Friends");
    socialMenu->addAction("Owl Mail");

    auto *systemMenu = menuBar()->addMenu("System");
    systemMenu->addAction("Settings");
    systemMenu->addAction("Logout");
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(10);

    m_campusTime = new CampusTimeWidget(central);
    m_actionBar = new LocationActionBar(central);

    auto *topLayout = new QHBoxLayout;
    topLayout->setSpacing(10);
    topLayout->addWidget(m_campusTime, 1);
    topLayout->addWidget(m_actionBar, 2);

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
    connect(m_actionBar, &LocationActionBar::moveToLocationRequested,
            this, &MainWindow::moveToLocationRequested);
    connect(m_courses, &CourseScheduleWidget::goToClassroomRequested,
            this, &MainWindow::joinClassRequested);
    connect(m_inventory, &InventoryWidget::itemUseRequested,
            this, &MainWindow::itemUseRequested);
    connect(m_inventory, &InventoryWidget::itemInspectRequested,
            this, &MainWindow::itemInspectRequested);
    connect(m_inventory, &InventoryWidget::itemGiftRequested,
            this, &MainWindow::itemGiftRequested);
}
