#include "ui/widgets/LocationActionBar.h"

#include <QHBoxLayout>
#include <QPushButton>

LocationActionBar::LocationActionBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("LocationActionBar");
    buildUi();
}

void LocationActionBar::buildUi()
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(8);

    auto *greatHall = new QPushButton("Great Hall", this);
    auto *classroom = new QPushButton("Classroom", this);
    auto *library = new QPushButton("Library", this);
    auto *courtyard = new QPushButton("Courtyard", this);
    auto *hogsmeade = new QPushButton("Hogsmeade", this);

    layout->addWidget(greatHall);
    layout->addWidget(classroom);
    layout->addWidget(library);
    layout->addWidget(courtyard);
    layout->addWidget(hogsmeade);
    layout->addStretch();

    connect(greatHall, &QPushButton::clicked, this, [this] {
        emit moveToLocationRequested("great_hall");
    });
    connect(classroom, &QPushButton::clicked, this, [this] {
        emit moveToLocationRequested("transfiguration_classroom");
    });
    connect(library, &QPushButton::clicked, this, [this] {
        emit moveToLocationRequested("library");
        emit studyRequested("library");
    });
    connect(courtyard, &QPushButton::clicked, this, [this] {
        emit moveToLocationRequested("courtyard");
    });
    connect(hogsmeade, &QPushButton::clicked, this, [this] {
        emit joinActivityRequested("hogsmeade_weekend");
    });
}
