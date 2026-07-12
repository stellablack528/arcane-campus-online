#include "ui/widgets/CourseScheduleWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QVBoxLayout>

CourseScheduleWidget::CourseScheduleWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("CourseScheduleWidget");
    buildUi();
}

void CourseScheduleWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *title = new QLabel("Today Classes", this);
    title->setObjectName("PanelTitle");

    m_list = new QListWidget(this);
    m_list->setObjectName("CourseList");
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);

    addCourse("transfiguration", "08:00", "Transfiguration", "In Progress", "prof_mcgonagall");
    addCourse("charms", "10:00", "Charms", "Not Started", "prof_flitwick");
    addCourse("herbology", "14:00", "Herbology", "Not Started", "prof_garlick");
    addCourse("defense", "16:00", "Defense Against the Dark Arts", "Not Started", "prof_defense");

    auto *buttonLayout = new QHBoxLayout;
    auto *goButton = new QPushButton("Go to Classroom", this);
    auto *detailButton = new QPushButton("Details", this);
    buttonLayout->addWidget(goButton);
    buttonLayout->addWidget(detailButton);

    layout->addWidget(title);
    layout->addWidget(m_list, 1);
    layout->addLayout(buttonLayout);

    connect(goButton, &QPushButton::clicked, this, [this] {
        if (auto *item = m_list->currentItem()) {
            emit goToClassroomRequested(courseId(item));
        }
    });

    connect(detailButton, &QPushButton::clicked, this, [this] {
        if (auto *item = m_list->currentItem()) {
            emit courseDetailRequested(courseId(item));
        }
    });

    connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        emit goToClassroomRequested(courseId(item));
    });

    connect(m_list, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        auto *item = m_list->itemAt(pos);
        if (!item) {
            return;
        }

        QMenu menu(this);
        auto *details = menu.addAction("Course Details");
        auto *professor = menu.addAction("Professor Info");
        auto *chosen = menu.exec(m_list->mapToGlobal(pos));
        if (chosen == details) {
            emit courseDetailRequested(courseId(item));
        } else if (chosen == professor) {
            emit professorInfoRequested(professorId(item));
        }
    });
}

void CourseScheduleWidget::addCourse(const QString &id,
                                     const QString &time,
                                     const QString &name,
                                     const QString &status,
                                     const QString &professorId)
{
    auto *item = new QListWidgetItem(QString("%1  %2  -  %3").arg(time, name, status), m_list);
    item->setData(Qt::UserRole, id);
    item->setData(Qt::UserRole + 1, professorId);
}

QString CourseScheduleWidget::courseId(QListWidgetItem *item) const
{
    return item ? item->data(Qt::UserRole).toString() : QString();
}

QString CourseScheduleWidget::professorId(QListWidgetItem *item) const
{
    return item ? item->data(Qt::UserRole + 1).toString() : QString();
}
