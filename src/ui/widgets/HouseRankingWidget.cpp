#include "ui/widgets/HouseRankingWidget.h"

#include <QFont>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

HouseRankingWidget::HouseRankingWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("HouseRankingWidget");
    buildUi();
}

void HouseRankingWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    auto *title = new QLabel("House Cup", this);
    title->setObjectName("PanelTitle");

    m_list = new QListWidget(this);
    m_list->setObjectName("RankingList");

    struct Ranking { QString emoji; QString name; int points; };
    const Ranking rankings[] = {
        {"🦅", "Ravenclaw",  145},
        {"🦁", "Gryffindor", 132},
        {"🦡", "Hufflepuff", 118},
        {"🐍", "Slytherin",  109},
    };
    for (const auto &r : rankings) {
        auto *item = new QListWidgetItem(
            QString("%1  %2  —  %3 pts").arg(r.emoji, r.name).arg(r.points, 3),
            m_list);
        QFont font = item->font();
        font.setPointSize(12);
        item->setFont(font);
        item->setSizeHint(QSize(0, 32));
    }

    layout->addWidget(title);
    layout->addWidget(m_list, 1);
}
