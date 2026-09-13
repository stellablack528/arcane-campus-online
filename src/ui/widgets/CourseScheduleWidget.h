#pragma once

#include <QString>
#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;

struct CourseSeed {
    QString id;
    QString time;
    QString nameKey;
    QString statusKey;
    QString professorId;
};

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
    void retranslateUi();
    QString courseId(QListWidgetItem *item) const;
    QString professorId(QListWidgetItem *item) const;

    QLabel *m_titleLabel = nullptr;
    QListWidget *m_list = nullptr;
    QPushButton *m_goButton = nullptr;
    QPushButton *m_detailButton = nullptr;
    QList<CourseSeed> m_seed;
};