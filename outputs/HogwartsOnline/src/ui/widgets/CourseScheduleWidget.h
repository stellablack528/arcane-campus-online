#pragma once

#include <QString>
#include <QWidget>

class QListWidget;
class QListWidgetItem;

class CourseScheduleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CourseScheduleWidget(QWidget *parent = nullptr);

signals:
    void goToClassroomRequested(const QString &courseId);
    void courseDetailRequested(const QString &courseId);
    void professorInfoRequested(const QString &professorId);

private:
    void buildUi();
    void addCourse(const QString &id,
                   const QString &time,
                   const QString &name,
                   const QString &status,
                   const QString &professorId);
    QString courseId(QListWidgetItem *item) const;
    QString professorId(QListWidgetItem *item) const;

    QListWidget *m_list = nullptr;
};
