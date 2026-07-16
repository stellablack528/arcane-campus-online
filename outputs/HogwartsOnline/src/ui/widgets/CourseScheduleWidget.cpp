#include "ui/widgets/CourseScheduleWidget.h"
#include "core/LanguageManager.h"

#include <QEvent>
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
    retranslateUi();
}

void CourseScheduleWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("PanelTitle");

    m_list = new QListWidget(this);
    m_list->setObjectName("CourseList");
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);

    auto *buttonLayout = new QHBoxLayout;
    m_goButton = new QPushButton(this);
    m_detailButton = new QPushButton(this);
    buttonLayout->addWidget(m_goButton);
    buttonLayout->addWidget(m_detailButton);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_list, 1);
    layout->addLayout(buttonLayout);

    connect(m_goButton, &QPushButton::clicked, this, [this] {
        if (auto *item = m_list->currentItem()) {
            emit goToClassroomRequested(courseId(item));
        }
    });

    connect(m_detailButton, &QPushButton::clicked, this, [this] {
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

        auto &lm = LanguageManager::instance();
        const bool isCn = (lm.currentLanguage() == Language::Chinese);
        QMenu menu(this);
        auto *details = menu.addAction(isCn ? "课程详情" : "Course Details");
        auto *professor = menu.addAction(isCn ? "教授信息" : "Professor Info");
        auto *chosen = menu.exec(m_list->mapToGlobal(pos));
        if (chosen == details) {
            emit courseDetailRequested(courseId(item));
        } else if (chosen == professor) {
            emit professorInfoRequested(professorId(item));
        }
    });
}

void CourseScheduleWidget::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    m_titleLabel->setText(isCn ? "今日课程" : "Today Classes");
    m_goButton->setText(isCn ? "前往教室" : "Go to Classroom");
    m_detailButton->setText(isCn ? "详情" : "Details");
    rebuildCourses();
}

void CourseScheduleWidget::rebuildCourses()
{
    if (!m_list) return;
    m_list->clear();
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    if (isCn) {
        addCourse("transfiguration", "08:00", "变形术", "进行中", "prof_mcgonagall");
        addCourse("charms", "10:00", "魔咒学", "未开始", "prof_flitwick");
        addCourse("herbology", "14:00", "草药学", "未开始", "prof_garlick");
        addCourse("defense", "16:00", "黑魔法防御术", "未开始", "prof_defense");
    } else {
        addCourse("transfiguration", "08:00", "Transfiguration", "In Progress", "prof_mcgonagall");
        addCourse("charms", "10:00", "Charms", "Not Started", "prof_flitwick");
        addCourse("herbology", "14:00", "Herbology", "Not Started", "prof_garlick");
        addCourse("defense", "16:00", "Defense Against the Dark Arts", "Not Started", "prof_defense");
    }
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

void CourseScheduleWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
