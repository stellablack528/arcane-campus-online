#include <QApplication>
#include <QFile>
#include <QFont>
#include <QIODevice>

#include "ui/LoginWindow.h"
#include "ui/MainWindow.h"
#include "ui/EnrollmentDialog.h"
#include "application/controller/CampusController.hpp"
#include "network/CampusWireClient.hpp"

#include "DBConnection.hpp"
#include "DatabaseConfigLoader.hpp"
#include "UserDAO.hpp"
#include "CharacterDAO.hpp"
#include "InventoryDAO.hpp"
#include "LocationDAO.hpp"
#include "MessageDAO.hpp"
#include "NPCDAO.hpp"
#include "RoomDAO.hpp"
#include "FriendDAO.hpp"
#include "StoryDAO.hpp"

#include <filesystem>
#include <memory>

namespace {

// New-student stories require the database; keep the failure visible on the login screen.
std::shared_ptr<arcane::database::DBConnection> tryCreateConnection(QString* failure)
{
    std::filesystem::path configPath;
    if (const char* path = std::getenv("ARCANE_DB_CONFIG")) {
        configPath = path;
    } else {
        configPath = std::filesystem::path("database.ini");
    }

    std::string error;
    const auto config = arcane::database::DatabaseConfigLoader::loadFromFile(configPath, &error);
    if (!config) {
        if (failure) *failure = QString::fromStdString(error);
        return nullptr;
    }

    auto connection = std::make_shared<arcane::database::DBConnection>(*config);
    if (!connection->connect()) {
        if (failure) *failure = QString::fromStdString(connection->lastError());
        return nullptr;
    }
    return connection;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Hogwarts Online");
    QApplication::setOrganizationName("FanProject");

    QFont appFont("Georgia", 10);
    app.setFont(appFont);

    QFile styleFile(":/qss/dark_wood.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    auto *loginWindow = new LoginWindow;
    auto *mainWindow = new MainWindow;
    auto *campusController = new arcane::application::controller::CampusController(&app);

    QString databaseError;
    if (auto connection = tryCreateConnection(&databaseError)) {
        auto userDao = std::make_shared<arcane::database::UserDAO>(connection);
        auto characterDao = std::make_shared<arcane::database::CharacterDAO>(connection);
        auto messageDao = std::make_shared<arcane::database::MessageDAO>(connection);
        auto inventoryDao = std::make_shared<arcane::database::InventoryDAO>(connection);
        auto locationDao = std::make_shared<arcane::database::LocationDAO>(connection);
        auto roomDao = std::make_shared<arcane::database::RoomDAO>(connection);
        auto npcDao = std::make_shared<arcane::database::NPCDAO>(connection);
        auto friendDao = std::make_shared<arcane::database::FriendDAO>(connection);
        auto storyDao = std::make_shared<arcane::database::StoryDAO>(connection);

        campusController->configureSessionService(userDao, characterDao, inventoryDao);
        campusController->configureChatService(messageDao);
        campusController->configureInventoryService(inventoryDao);
        campusController->configureMapService(inventoryDao, locationDao);
        campusController->configureCampusService(roomDao, npcDao);
        campusController->configureSocialService(friendDao, characterDao);
        campusController->configureStoryProgressService(storyDao, characterDao);
        campusController->configureStoryPersistence(connection);
    } else if (qEnvironmentVariable("ARCANE_DEMO_MODE") != QLatin1String("1")) {
        loginWindow->setDatabaseAvailable(false, databaseError);
    }

    QObject::connect(loginWindow, &LoginWindow::loginRequested,
                     campusController, &arcane::application::controller::CampusController::handleLogin);
    QObject::connect(loginWindow, &LoginWindow::newStudentRequested,
                     campusController, [loginWindow, campusController]() {
        EnrollmentDialog dialog(loginWindow);
        if (dialog.exec() == QDialog::Accepted) {
            campusController->handleEnrollment(dialog.buildRequest());
        }
    });
    QObject::connect(campusController, &arcane::application::controller::CampusController::loginAccepted,
                     mainWindow, [loginWindow, mainWindow](const QString &studentName,
                                                           const QString &houseName) {
        mainWindow->setPlayerIdentity(studentName, houseName);
        mainWindow->show();
        loginWindow->close();
        loginWindow->deleteLater();
    });

    auto* liveClient = new arcane::network::CampusWireClient(&app);
    auto liveName = std::make_shared<QString>();
    QObject::connect(campusController, &arcane::application::controller::CampusController::loginAccepted,
                     &app, [liveName](const QString& name, const QString&) { *liveName = name; });
    QObject::connect(campusController, &arcane::application::controller::CampusController::playerLocationChanged,
                     &app, [liveClient, liveName](const QString& location, const QString& state) {
        if (state == QLatin1String("Free exploration") && !liveClient->connected() && !liveName->isEmpty())
            liveClient->join(*liveName, location);
        else if (liveClient->connected()) liveClient->move(location);
    });
    QObject::connect(liveClient, &arcane::network::CampusWireClient::messageReceived,
                     mainWindow, &MainWindow::appendCampusMessage);
    QObject::connect(mainWindow, &MainWindow::sendChatRequested,
                     campusController, [campusController, liveClient](const QString& channel, const QString& text) {
        if (text.startsWith(QStringLiteral("/owl "))) {
            const auto parts = text.mid(5).split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() >= 2 && liveClient->connected())
                liveClient->owl(parts.first(), text.mid(5 + parts.first().size() + 1));
            return;
        }
        if (liveClient->connected() && !campusController->hasActiveStory()
            && (channel == QLatin1String("Location") || channel == QStringLiteral("地点"))) {
            if (text == QLatin1String("去图书馆") || text == QLatin1String("去礼堂")
                || text == QLatin1String("去庭院") || text.startsWith(QLatin1String("go to "))
                || text.contains(QStringLiteral("翻页")) || text.contains(QStringLiteral("罗温"))
                || text.contains(QStringLiteral("旧书")) || text.contains(QStringLiteral("教授"))
                || text.contains(QStringLiteral("米拉")) || text.contains(QStringLiteral("管理员"))
                || text.contains(QStringLiteral("调查")) || text.contains(QStringLiteral("喷泉"))
                || text.contains(QStringLiteral("刻字"))
                || text.compare(QStringLiteral("investigate book"), Qt::CaseInsensitive) == 0)
                campusController->handleChat(channel, text);
            else liveClient->say(text);
            return;
        }
        campusController->handleChat(channel, text);
    });
    QObject::connect(mainWindow, &MainWindow::moveToLocationRequested,
                     campusController, &arcane::application::controller::CampusController::handleMove);
    QObject::connect(mainWindow, &MainWindow::joinClassRequested,
                     campusController, &arcane::application::controller::CampusController::handleCourseJoin);
    QObject::connect(mainWindow, &MainWindow::itemUseRequested,
                     campusController, &arcane::application::controller::CampusController::handleItemUse);
    QObject::connect(mainWindow, &MainWindow::itemInspectRequested,
                     campusController, &arcane::application::controller::CampusController::handleItemInspect);
    QObject::connect(mainWindow, &MainWindow::itemGiftRequested,
                     campusController, &arcane::application::controller::CampusController::handleItemGift);
    QObject::connect(mainWindow, &MainWindow::courseDetailsRequested,
                     campusController, &arcane::application::controller::CampusController::handleCourseDetails);
    QObject::connect(mainWindow, &MainWindow::professorInfoRequested,
                     campusController, &arcane::application::controller::CampusController::handleProfessorInfo);
    QObject::connect(mainWindow, &MainWindow::studyRequested,
                     campusController, &arcane::application::controller::CampusController::handleStudy);
    QObject::connect(mainWindow, &MainWindow::activityJoinRequested,
                     campusController, &arcane::application::controller::CampusController::handleActivity);
    QObject::connect(mainWindow, &MainWindow::memberSelected,
                     campusController, &arcane::application::controller::CampusController::handleMemberSelected);
    QObject::connect(mainWindow, &MainWindow::privateChatRequested,
                     campusController, &arcane::application::controller::CampusController::handlePrivateChat);
    QObject::connect(mainWindow, &MainWindow::profileRequested,
                     campusController, &arcane::application::controller::CampusController::handleProfile);
    QObject::connect(mainWindow, &MainWindow::useMaraudersMapRequested,
                     campusController, &arcane::application::controller::CampusController::handleUseMaraudersMap);
    QObject::connect(mainWindow, &MainWindow::refreshInventoryRequested,
                     campusController, &arcane::application::controller::CampusController::handleRefreshInventory);
    QObject::connect(mainWindow, &MainWindow::timePeriodChanged,
                     campusController, &arcane::application::controller::CampusController::onTimePeriodChanged);
    QObject::connect(mainWindow, &MainWindow::nightPatrolRequested,
                     campusController, &arcane::application::controller::CampusController::handleStartNightPatrol);
    QObject::connect(campusController, &arcane::application::controller::CampusController::housePointsChanged,
                     mainWindow, &MainWindow::onHousePointsChanged);
    // 剧情模块接线
    QObject::connect(mainWindow, &MainWindow::storyChoiceRequested,
                     campusController, &arcane::application::controller::CampusController::handleMakeChoice);
    QObject::connect(mainWindow, &MainWindow::reputationQueryRequested,
                     campusController, &arcane::application::controller::CampusController::handleQueryReputation);
    QObject::connect(campusController, &arcane::application::controller::CampusController::richCampusMessageProduced,
                     mainWindow, &MainWindow::appendRichCampusMessage);
    // 开局剧情接线
    QObject::connect(mainWindow, &MainWindow::prologueChoiceSelected,
                     campusController, &arcane::application::controller::CampusController::handlePrologueChoice);
    QObject::connect(campusController, &arcane::application::controller::CampusController::sceneHeaderChanged,
                     mainWindow, &MainWindow::setSceneHeader);
    QObject::connect(campusController, &arcane::application::controller::CampusController::inputHintChanged,
                     mainWindow, &MainWindow::setInputHint);
    QObject::connect(campusController, &arcane::application::controller::CampusController::choicesPresented,
                     mainWindow, &MainWindow::presentChoices);

    QObject::connect(campusController, &arcane::application::controller::CampusController::campusMessageProduced,
                     mainWindow, &MainWindow::appendCampusMessage);
    QObject::connect(campusController, &arcane::application::controller::CampusController::playerLocationChanged,
                     mainWindow, &MainWindow::updatePlayerLocation);
    QObject::connect(campusController, &arcane::application::controller::CampusController::playerGoldChanged,
                     mainWindow, &MainWindow::updatePlayerGold);
    QObject::connect(campusController, &arcane::application::controller::CampusController::feedbackProduced,
                     mainWindow, &MainWindow::showFeedback);
    QObject::connect(campusController, &arcane::application::controller::CampusController::feedbackProduced,
                     loginWindow, &LoginWindow::showFeedback);
    QObject::connect(campusController, &arcane::application::controller::CampusController::maraudersMapRevealed,
                     mainWindow, &MainWindow::displayMaraudersMap);
    QObject::connect(campusController, &arcane::application::controller::CampusController::inventoryRefreshed,
                     mainWindow, &MainWindow::refreshInventory);

    loginWindow->show();

    return app.exec();
}
