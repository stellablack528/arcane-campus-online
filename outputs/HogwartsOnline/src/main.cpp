#include <QApplication>
#include <QFile>
#include <QFont>
#include <QIODevice>

#include "ui/WelcomeWindow.h"
#include "ui/LoginWindow.h"
#include "ui/MainWindow.h"
#include "core/LanguageManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Hogwarts Online");
    QApplication::setOrganizationName("FanProject");

    // 全局字体
    QFont appFont("Georgia", 10);
    app.setFont(appFont);

    // 加载暗色木质主题
    QFile styleFile(":/qss/dark_wood.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    // 三个窗口：欢迎 → 登录 → 主界面
    auto *welcomeWindow = new WelcomeWindow;
    auto *loginWindow = new LoginWindow;
    auto *mainWindow = new MainWindow;

    // Welcome → Login
    QObject::connect(welcomeWindow, &WelcomeWindow::startRequested,
                     welcomeWindow, [welcomeWindow, loginWindow]() {
        loginWindow->show();
        welcomeWindow->close();
        welcomeWindow->deleteLater();
    });

    // Login → Main
    QObject::connect(loginWindow, &LoginWindow::loginRequested,
                     loginWindow, [loginWindow, mainWindow]
                     (const QString &studentName, const QString &houseName,
                      const WandInfo &wand) {
        mainWindow->setPlayerIdentity(studentName, houseName);
        mainWindow->setWandInfo(wand.wood, wand.core, wand.lengthInches, wand.flexibility);
        mainWindow->show();
        loginWindow->close();
        loginWindow->deleteLater();
    });

    welcomeWindow->show();

    return app.exec();
}
