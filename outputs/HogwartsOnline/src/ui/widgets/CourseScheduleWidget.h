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
    void retranslateUi();

signals:
    void goToClassroomRequested(const QString &courseId);
    void courseDetailRequested(const QString &courseId);
    void professorInfoRequested(const QString &professorId);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void rebuildCourses();
    void addCourse(const QString &id,
                   const QString &time,
                   const QString &name,
                   const QString &status,
                   const QString &professorId);
    QString courseId(QListWidgetItem *item) const;
    QString professorId(QListWidgetItem *item) const;

    QLabel *m_titleLabel = nullptr;
    QListWidget *m_list = nullptr;
    QPushButton *m_goButton = nullptr;
    QPushButton *m_detailButton = nullptr;
};
