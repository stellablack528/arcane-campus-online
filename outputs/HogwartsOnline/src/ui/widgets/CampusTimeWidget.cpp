#include "ui/widgets/CampusTimeWidget.h"
#include "core/LanguageManager.h"

#include <QDateTime>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>

CampusTimeWidget::CampusTimeWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("CampusTimeWidget");
    buildUi();
    retranslateUi();
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
    m_termLabel = new QLabel(this);
    m_termLabel->setObjectName("TimeSecondary");
    m_weatherLabel = new QLabel(this);
    m_weatherLabel->setObjectName("TimeSecondary");
    m_eventLabel = new QLabel(this);
    m_eventLabel->setObjectName("TimeSecondary");

    layout->addWidget(m_weekdayLabel, 0, 0);
    layout->addWidget(m_timeLabel, 0, 1);
    layout->addWidget(m_termLabel, 1, 0);
    layout->addWidget(m_weatherLabel, 1, 1);
    layout->addWidget(m_eventLabel, 2, 0, 1, 2);
}

void CampusTimeWidget::refreshTime()
{
    auto &lm = LanguageManager::instance();
    const QDateTime now = QDateTime::currentDateTime();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    // 星期（手动翻译以避免 locale 差异）
    static const QString weekdaysEn[7] = {
        "Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"
    };
    static const QString weekdaysCn[7] = {
        "星期一","星期二","星期三","星期四","星期五","星期六","星期日"
    };
    const int dow = now.date().dayOfWeek(); // 1..7
    m_weekdayLabel->setText(isCn ? weekdaysCn[dow - 1] : weekdaysEn[dow - 1]);

    m_timeLabel->setText(now.toString(isCn ? "hh:mm" : "hh:mm AP"));
}

void CampusTimeWidget::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);
    m_termLabel->setText(isCn ? "秋学期 · 第三周" : "Autumn Term · Week Three");
    m_weatherLabel->setText(isCn ? "天气：晴朗" : "Weather: Sunny");
    m_eventLabel->setText(isCn ? "当前事件：早餐时间" : "Current Event: Breakfast");
    refreshTime();
}

void CampusTimeWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
