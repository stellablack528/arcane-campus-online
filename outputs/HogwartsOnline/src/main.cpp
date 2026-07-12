#include <QApplication>
#include <QFile>
#include <QFont>
#include <QIODevice>

#include "ui/LoginWindow.h"
#include "ui/MainWindow.h"

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

    QObject::connect(loginWindow, &LoginWindow::loginRequested,
                     mainWindow, [loginWindow, mainWindow](const QString &studentName,
                                                           const QString &houseName) {
        mainWindow->setPlayerIdentity(studentName, houseName);
        mainWindow->show();
        loginWindow->close();
        loginWindow->deleteLater();
    });

    loginWindow->show();

    return app.exec();
}
