#include "ui/widgets/CampusTimeWidget.h"

#include <QDateTime>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>

CampusTimeWidget::CampusTimeWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("CampusTimeWidget");
    buildUi();
    refreshTime();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CampusTimeWidget::refreshTime);
    m_timer->start(1000);
}

void CampusTimeWidget::buildUi()
{
    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setHorizontalSpacing(16);
    layout->setVerticalSpacing(4);

    m_weekdayLabel = new QLabel(this);
    m_weekdayLabel->setObjectName("TimePrimary");
    m_timeLabel = new QLabel(this);
    m_timeLabel->setObjectName("TimePrimary");
    m_termLabel = new QLabel("Autumn Term - Week Three", this);
    m_weatherLabel = new QLabel("Weather: Sunny", this);
    m_eventLabel = new QLabel("Current Event: Breakfast", this);

    layout->addWidget(m_weekdayLabel, 0, 0);
    layout->addWidget(m_timeLabel, 0, 1);
    layout->addWidget(m_termLabel, 1, 0);
    layout->addWidget(m_weatherLabel, 1, 1);
    layout->addWidget(m_eventLabel, 2, 0, 1, 2);
}

void CampusTimeWidget::refreshTime()
{
    const QDateTime now = QDateTime::currentDateTime();
    m_weekdayLabel->setText(now.toString("dddd"));
    m_timeLabel->setText(now.toString("hh:mm AP"));
}
