#pragma once

#include <QWidget>

class QLabel;
class QTimer;

class CampusTimeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CampusTimeWidget(QWidget *parent = nullptr);

signals:
    // 当校园时段发生变化时发出。
    // 参数是 arcane::application::world::TimePeriod 的 int 表示（用 int 避免跨线程时 Qt 元类型注册的麻烦）。
    void timePeriodChanged(int period);

private slots:
    void refreshTime();

private:
    void buildUi();

    QLabel *m_weekdayLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    QLabel *m_termLabel = nullptr;
    QLabel *m_weatherLabel = nullptr;
    QLabel *m_periodLabel = nullptr;
    QTimer *m_timer = nullptr;
    int m_lastPeriod = -1;  // -1 表示尚未初始化，避免启动即触发事件
};
