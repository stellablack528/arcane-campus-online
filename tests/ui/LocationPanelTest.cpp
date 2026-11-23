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
    auto* people = panel.findChild<QListWidget*>("PeopleHereList");
    if (!people || people->count() != 1
        || people->item(0)->text() != QStringLiteral("这里暂时没有可交谈的人。")) {
        std::cerr << "Empty scene does not show a translated placeholder\n";
        return 1;
    }
    panel.setEraId(QStringLiteral("riddle"));
    panel.setCurrentLocation(QStringLiteral("great_hall"));
    if (people->count() != 1 || people->item(0)->text() != QStringLiteral("伊芙琳·摩尔")) {
        std::cerr << "Historical hall cast is incorrect\n";
        return 1;
    }
    panel.setCurrentLocation(QStringLiteral("library"));
    if (people->count() != 1 || people->item(0)->flags() != Qt::NoItemFlags) {
        std::cerr << "Historical era exposes a later-generation classmate\n";
        return 1;
    }
    panel.setCurrentLocation(QStringLiteral("charms_classroom"));
    if (people->count() != 2 || people->item(0)->text() != QStringLiteral("艾尔德里奇教授")) {
        std::cerr << "Historical classroom cast is incorrect\n";
        return 1;
    }
    panel.setCurrentLocation(QStringLiteral("library"));
    panel.setSceneNpcs({{QStringLiteral("riddle.tom_riddle"), QStringLiteral("汤姆·里德尔")}}, {});
    QString member;
    QObject::connect(&panel, &LocationPanel::memberSelected, &panel, [&](const QString& id) { member = id; });
    if (people->count() != 1 || people->item(0)->text() != QStringLiteral("汤姆·里德尔")) return 1;
    emit people->itemClicked(people->item(0));
    if (member != QLatin1String("riddle.tom_riddle")) return 1;
    panel.setEraId(QStringLiteral("harry"));
    panel.setSceneNpcs({}, {QStringLiteral("npc_hermione")});
    if (people->count() != 1 || people->item(0)->flags() != Qt::NoItemFlags) return 1;
    panel.setSceneNpcs({{QStringLiteral("harry.hermione_granger"), QStringLiteral("赫敏·格兰杰")}},
                       {QStringLiteral("npc_hermione")});
    if (people->count() != 1 || people->item(0)->data(Qt::UserRole).toString() != QLatin1String("harry.hermione_granger")) return 1;
    return 0;
}
