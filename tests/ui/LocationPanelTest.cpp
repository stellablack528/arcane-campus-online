#include "ui/widgets/LocationPanel.h"

#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>

#include <iostream>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    LocationPanel panel;
    panel.resize(300, 500);
    panel.show();
    app.processEvents();

    auto* scroll = panel.findChild<QScrollArea*>("LocationScrollArea");
    if (!scroll || !scroll->widget()) {
        std::cerr << "Location drawer is missing its scrollable content\n";
        return 1;
    }
    const auto sections = scroll->widget()->findChildren<QGroupBox*>(Qt::FindDirectChildrenOnly);
    if (sections.size() != 4) {
        std::cerr << "Expected location, actions, travel and people sections\n";
        return 1;
    }
    for (qsizetype i = 1; i < sections.size(); ++i) {
        if (sections[i - 1]->geometry().bottom() >= sections[i]->geometry().top()) {
            std::cerr << "Location drawer sections overlap\n";
            return 1;
        }
    }

    QString destination;
    QObject::connect(&panel, &LocationPanel::moveToLocationRequested,
                     &panel, [&](const QString& id) { destination = id; });
    panel.setCurrentLocation(QStringLiteral("transfiguration_classroom"));
    app.processEvents();
    bool foundClassroom = false;
    bool foundHogsmeade = false;
    for (auto* button : panel.findChildren<QPushButton*>("LocationMoveBtn")) {
        if (button->text() == QStringLiteral("变形术教室")) {
            foundClassroom = true;
            if (button->isEnabled()) return 1;
        }
        if (button->text() == QStringLiteral("霍格莫德")) {
            foundHogsmeade = true;
            button->click();
        }
    }
    if (!foundClassroom || !foundHogsmeade || destination != QLatin1String("hogsmeade")) {
        std::cerr << "Placeholder destinations are not reachable from the drawer\n";
        return 1;
    }

    panel.setCurrentLocation(QStringLiteral("hogsmeade"));
    const auto* people = panel.findChild<QListWidget*>("PeopleHereList");
    if (!people || people->count() != 1
        || people->item(0)->text() != QStringLiteral("这里暂时没有可交谈的人。")) {
        std::cerr << "Empty scene does not show a translated placeholder\n";
        return 1;
    }
    return 0;
}
