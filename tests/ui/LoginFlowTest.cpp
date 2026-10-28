#include "ui/LoginWindow.h"

#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QString>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    LoginWindow login;
    const auto buttons = login.findChildren<QPushButton*>();
    const auto name = login.findChild<QLineEdit*>();
    if (buttons.size() != 2 || !name || !name->text().isEmpty()) return 1;

    int loginRequests = 0;
    int enrollRequests = 0;
    QString selectedHouse;
    QObject::connect(&login, &LoginWindow::loginRequested,
                     [&loginRequests, &selectedHouse](const QString&, const QString& house) {
                         ++loginRequests;
                         selectedHouse = house;
                     });
    QObject::connect(&login, &LoginWindow::newStudentRequested,
                     [&enrollRequests] { ++enrollRequests; });

    login.setDatabaseAvailable(false, QStringLiteral("missing database.ini"));
    for (auto* button : buttons) if (button->isEnabled()) return 2;
    if (!QMetaObject::invokeMethod(&login, "onLoginClicked") || loginRequests != 0) return 3;

    login.setDatabaseAvailable(true);
    for (auto* button : buttons) if (!button->isEnabled()) return 4;
    name->setText(QStringLiteral("new_student_test"));
    if (!QMetaObject::invokeMethod(&login, "onLoginClicked") || loginRequests != 1
        || selectedHouse != QLatin1String("Ravenclaw")) return 5;
    for (auto* button : buttons) {
        if (button->text().contains(QStringLiteral("Enroll"), Qt::CaseInsensitive)
            || button->text().contains(QStringLiteral("新生"))) button->click();
    }
    return enrollRequests == 1 ? 0 : 6;
}
