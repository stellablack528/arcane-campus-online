#pragma once

#include <QWidget>

class QLabel;
class QTimer;

class CampusTimeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CampusTimeWidget(QWidget *parent = nullptr);

private slots:
    void refreshTime();

private:
    void buildUi();

    QLabel *m_weekdayLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    QLabel *m_termLabel = nullptr;
    QLabel *m_weatherLabel = nullptr;
    QLabel *m_eventLabel = nullptr;
    QTimer *m_timer = nullptr;
};
