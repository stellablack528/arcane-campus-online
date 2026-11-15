#include "ui/widgets/CourseScheduleWidget.h"

#include "ui/I18n.hpp"

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

    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) { retranslateUi(); });
}

void CourseScheduleWidget::setTermSchedule(int week, int weekday, int morningSubject, int afternoonSubject)
{
    m_termActive = true;
    m_termWeek = week;
    m_termWeekday = weekday;
    m_morningSubject = morningSubject;
    m_afternoonSubject = afternoonSubject;
    retranslateUi();
}

void CourseScheduleWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

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
        QMenu menu(this);
        auto *details = menu.addAction(TR("class.details2"));
        auto *professor = menu.addAction(TR("class.professor"));
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
    if (m_titleLabel)   m_titleLabel->setText(TR("title.classes"));
    if (m_goButton)     m_goButton->setText(TR("class.goto"));
    if (m_detailButton) m_detailButton->setText(TR("class.details"));

    m_list->clear();
    if (!m_termActive) {
        auto *item = new QListWidgetItem(QStringLiteral("完成入学剧情后开放课表。"), m_list);
        item->setFlags(Qt::NoItemFlags);
        return;
    }
    if (m_termActive) {
        if (m_termWeekday < 0) {
            m_titleLabel->setText(QStringLiteral("第一学期前五周已完成"));
            auto *item = new QListWidgetItem(QStringLiteral("后续课程尚未开放。"), m_list);
            item->setFlags(Qt::NoItemFlags);
            return;
        }
        const QString day = QStringList{QStringLiteral("一"), QStringLiteral("二"),
            QStringLiteral("三"), QStringLiteral("四"), QStringLiteral("五"),
            QStringLiteral("六"), QStringLiteral("日")}.value(m_termWeekday);
        m_titleLabel->setText(QStringLiteral("第%1周 · 周%2课程").arg(m_termWeek).arg(day));
        const QStringList ids{QStringLiteral("charms"), QStringLiteral("transfiguration"),
            QStringLiteral("herbology"), QStringLiteral("defense")};
        const QStringList names{QStringLiteral("魔咒学"), QStringLiteral("变形术"),
            QStringLiteral("草药学"), QStringLiteral("黑魔法防御术")};
        const auto addCourse = [&](const QString& time, int subject) {
            if (subject < 0 || subject >= ids.size()) return;
            auto *item = new QListWidgetItem(time + QStringLiteral("  ") + names.at(subject), m_list);
            item->setData(Qt::UserRole, ids.at(subject));
            item->setData(Qt::UserRole + 1, QString());
        };
        addCourse(QStringLiteral("09:00"), m_morningSubject);
        addCourse(QStringLiteral("14:00"), m_afternoonSubject);
        if (m_list->count() == 0) {
            auto *item = new QListWidgetItem(QStringLiteral("周末无课程，可自由探索和休息。"), m_list);
            item->setFlags(Qt::NoItemFlags);
        }
        return;
    }
}

QString CourseScheduleWidget::courseId(QListWidgetItem *item) const
{
    return item ? item->data(Qt::UserRole).toString() : QString();
}

QString CourseScheduleWidget::professorId(QListWidgetItem *item) const
{
    return item ? item->data(Qt::UserRole + 1).toString() : QString();
}
