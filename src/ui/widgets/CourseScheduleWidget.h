#pragma once

#include <QString>
#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;

class CourseScheduleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CourseScheduleWidget(QWidget *parent = nullptr);
    void setTermSchedule(int week, int weekday, int morningSubject, int afternoonSubject);

signals:
    void goToClassroomRequested(const QString &courseId);
    void courseDetailRequested(const QString &courseId);
    void professorInfoRequested(const QString &professorId);

private:
    void buildUi();
    void retranslateUi();
    QString courseId(QListWidgetItem *item) const;
    QString professorId(QListWidgetItem *item) const;

    QLabel *m_titleLabel = nullptr;
    QListWidget *m_list = nullptr;
    QPushButton *m_goButton = nullptr;
    QPushButton *m_detailButton = nullptr;
    bool m_termActive = false;
    int m_termWeek = 1;
    int m_termWeekday = 0;
    int m_morningSubject = -1;
    int m_afternoonSubject = -1;
};
