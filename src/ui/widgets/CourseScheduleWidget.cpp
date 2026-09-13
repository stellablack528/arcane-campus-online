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

    m_seed.clear();
    m_seed.append({"transfiguration", "08:00", "class.transfiguration", "class.inprogress", "prof_mcgonagall"});
    m_seed.append({"charms",          "10:00", "class.charms",          "class.notstarted", "prof_flitwick"});
    m_seed.append({"herbology",       "14:00", "class.herbology",       "class.notstarted", "prof_garlick"});
    m_seed.append({"defense",         "16:00", "class.defense",         "class.notstarted", "prof_defense"});

    retranslateUi();

    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) { retranslateUi(); });
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
    for (const auto& s : m_seed) {
        auto *item = new QListWidgetItem(
            QStringLiteral("%1  %2  -  %3").arg(s.time, TRQ(s.nameKey), TRQ(s.statusKey)),
            m_list);
        item->setData(Qt::UserRole, s.id);
        item->setData(Qt::UserRole + 1, s.professorId);
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