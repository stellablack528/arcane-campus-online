#include "ui/widgets/LocationActionBar.h"
#include "core/LanguageManager.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QPushButton>

LocationActionBar::LocationActionBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("LocationActionBar");
    buildUi();
    retranslateUi();
}

void LocationActionBar::buildUi()
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(8);

    m_greatHall = new QPushButton(this);
    m_classroom = new QPushButton(this);
    m_library = new QPushButton(this);
    m_courtyard = new QPushButton(this);
    m_hogsmeade = new QPushButton(this);

    layout->addWidget(m_greatHall);
    layout->addWidget(m_classroom);
    layout->addWidget(m_library);
    layout->addWidget(m_courtyard);
    layout->addWidget(m_hogsmeade);
    layout->addStretch();

    connect(m_greatHall, &QPushButton::clicked, this, [this] {
        emit moveToLocationRequested("great_hall");
    });
    connect(m_classroom, &QPushButton::clicked, this, [this] {
        emit moveToLocationRequested("transfiguration_classroom");
    });
    connect(m_library, &QPushButton::clicked, this, [this] {
        emit moveToLocationRequested("library");
        emit studyRequested("library");
    });
    connect(m_courtyard, &QPushButton::clicked, this, [this] {
        emit moveToLocationRequested("courtyard");
    });
    connect(m_hogsmeade, &QPushButton::clicked, this, [this] {
        emit joinActivityRequested("hogsmeade_weekend");
    });
}

void LocationActionBar::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);
    m_greatHall->setText(isCn ? "🏛 礼堂" : "🏛 Great Hall");
    m_classroom->setText(isCn ? "📚 教室" : "📚 Classroom");
    m_library->setText(isCn ? "📖 图书馆" : "📖 Library");
    m_courtyard->setText(isCn ? "🌳 庭院" : "🌳 Courtyard");
    m_hogsmeade->setText(isCn ? "🏘 霍格莫德" : "🏘 Hogsmeade");
}

void LocationActionBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
