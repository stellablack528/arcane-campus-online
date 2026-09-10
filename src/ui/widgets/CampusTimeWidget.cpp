#include "ui/widgets/CampusTimeWidget.h"

#include "application/world/WorldClock.hpp"

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
    // 每 30 秒检查一次时段变化（时段精度到小时即可，30 秒足够灵敏且节省 CPU）。
    m_timer->start(30000);
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
    m_periodLabel = new QLabel(this);
    m_periodLabel->setObjectName("TimePrimary");

    layout->addWidget(m_weekdayLabel, 0, 0);
    layout->addWidget(m_timeLabel, 0, 1);
    layout->addWidget(m_termLabel, 1, 0);
    layout->addWidget(m_weatherLabel, 1, 1);
    layout->addWidget(m_periodLabel, 2, 0, 1, 2);
}

void CampusTimeWidget::refreshTime()
{
    const QDateTime now = QDateTime::currentDateTime();
    m_weekdayLabel->setText(now.toString("dddd"));
    m_timeLabel->setText(now.toString("hh:mm AP"));

    using namespace arcane::application::world;
    const int hour = now.time().hour();
    const auto period = WorldClock::periodFromHour(hour);
    const int periodValue = static_cast<int>(period);

    m_periodLabel->setText(QStringLiteral("Period: ")
                           + QString::fromStdString(WorldClock::displayName(period)));

    // 只在跨时段时发信号，避免每 30 秒刷屏。
    if (m_lastPeriod != -1 && m_lastPeriod != periodValue) {
        emit timePeriodChanged(periodValue);
    }
    m_lastPeriod = periodValue;
}
