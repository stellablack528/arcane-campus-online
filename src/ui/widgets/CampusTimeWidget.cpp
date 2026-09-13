#include "ui/widgets/CampusTimeWidget.h"

#include "application/world/WorldClock.hpp"
#include "ui/I18n.hpp"

#include <QDateTime>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>

namespace {

QString weekdayKeyFrom(const QString& english)
{
    if (english == "Monday")    return "time.weekday.monday";
    if (english == "Tuesday")   return "time.weekday.tuesday";
    if (english == "Wednesday") return "time.weekday.wednesday";
    if (english == "Thursday")  return "time.weekday.thursday";
    if (english == "Friday")    return "time.weekday.friday";
    if (english == "Saturday")  return "time.weekday.saturday";
    if (english == "Sunday")    return "time.weekday.sunday";
    return {};
}

} // namespace

CampusTimeWidget::CampusTimeWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("CampusTimeWidget");
    buildUi();
    retranslateUi();
    refreshTime();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CampusTimeWidget::refreshTime);
    // 每 30 秒检查一次时段变化（时段精度到小时即可，30 秒足够灵敏且节省 CPU）。
    m_timer->start(30000);

    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) { retranslateUi(); refreshTime(); });
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
    m_termLabel = new QLabel(this);
    m_weatherLabel = new QLabel(this);
    m_periodLabel = new QLabel(this);
    m_periodLabel->setObjectName("TimePrimary");

    layout->addWidget(m_weekdayLabel, 0, 0);
    layout->addWidget(m_timeLabel, 0, 1);
    layout->addWidget(m_termLabel, 1, 0);
    layout->addWidget(m_weatherLabel, 1, 1);
    layout->addWidget(m_periodLabel, 2, 0, 1, 2);
}

void CampusTimeWidget::retranslateUi()
{
    m_termLabel->setText(TR("time.autumn"));
    m_weatherLabel->setText(TR("time.weather"));
}

void CampusTimeWidget::refreshTime()
{
    const QDateTime now = QDateTime::currentDateTime();
    const QString englishWeekday = now.toString("dddd");
    const QString weekdayKey = weekdayKeyFrom(englishWeekday);
    m_weekdayLabel->setText(weekdayKey.isEmpty() ? englishWeekday : TRQ(weekdayKey));
    m_timeLabel->setText(now.toString("hh:mm AP"));

    using namespace arcane::application::world;
    const int hour = now.time().hour();
    const auto period = WorldClock::periodFromHour(hour);
    const int periodValue = static_cast<int>(period);

    m_periodLabel->setText(TR2("time.period",
        {QString::fromStdString(WorldClock::displayName(period))}));

    // 只在跨时段时发信号，避免每 30 秒刷屏。
    if (m_lastPeriod != -1 && m_lastPeriod != periodValue) {
        emit timePeriodChanged(periodValue);
    }
    m_lastPeriod = periodValue;
}
