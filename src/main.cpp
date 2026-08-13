#include <QApplication>
#include <QFile>
#include <QFont>
#include <QIODevice>

#include "ui/LoginWindow.h"
#include "ui/MainWindow.h"
#include "application/controller/CampusController.hpp"

#include "DBConnection.hpp"
#include "DatabaseConfigLoader.hpp"
#include "UserDAO.hpp"
#include "CharacterDAO.hpp"
#include "InventoryDAO.hpp"
#include "LocationDAO.hpp"
#include "MessageDAO.hpp"

#include <filesystem>
#include <memory>

namespace {

// Attempts to build a live database connection from database.ini (or the env var
// ARCANE_DB_CONFIG). Returns nullptr when no database is configured, in which case
// the application falls back to demo mode.
std::shared_ptr<arcane::database::DBConnection> tryCreateConnection()
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
        return nullptr;
    }

    auto connection = std::make_shared<arcane::database::DBConnection>(*config);
    if (!connection->connect()) {
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

    // Wire the live database when a connection is available; otherwise stay in demo mode.
    if (auto connection = tryCreateConnection()) {
        auto userDao = std::make_shared<arcane::database::UserDAO>(connection);
        auto characterDao = std::make_shared<arcane::database::CharacterDAO>(connection);
        auto messageDao = std::make_shared<arcane::database::MessageDAO>(connection);
        auto inventoryDao = std::make_shared<arcane::database::InventoryDAO>(connection);
        auto locationDao = std::make_shared<arcane::database::LocationDAO>(connection);

        campusController->configureSessionService(userDao, characterDao);
        campusController->configureChatService(messageDao);
        campusController->configureInventoryService(inventoryDao);
        campusController->configureMapService(inventoryDao, locationDao);
    }

    QObject::connect(loginWindow, &LoginWindow::loginRequested,
                     campusController, &arcane::application::controller::CampusController::handleLogin);
    QObject::connect(campusController, &arcane::application::controller::CampusController::loginAccepted,
                     mainWindow, [loginWindow, mainWindow](const QString &studentName,
                                                           const QString &houseName) {
        mainWindow->setPlayerIdentity(studentName, houseName);
        mainWindow->show();
        loginWindow->close();
        loginWindow->deleteLater();
    });

    QObject::connect(mainWindow, &MainWindow::sendChatRequested,
                     campusController, &arcane::application::controller::CampusController::handleChat);
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

    QObject::connect(campusController, &arcane::application::controller::CampusController::campusMessageProduced,
                     mainWindow, &MainWindow::appendCampusMessage);
    QObject::connect(campusController, &arcane::application::controller::CampusController::playerLocationChanged,
                     mainWindow, &MainWindow::updatePlayerLocation);
    QObject::connect(campusController, &arcane::application::controller::CampusController::feedbackProduced,
                     mainWindow, &MainWindow::showFeedback);
    QObject::connect(campusController, &arcane::application::controller::CampusController::maraudersMapRevealed,
                     mainWindow, &MainWindow::displayMaraudersMap);
    QObject::connect(campusController, &arcane::application::controller::CampusController::inventoryRefreshed,
                     mainWindow, &MainWindow::refreshInventory);

    loginWindow->show();

    return app.exec();
}
