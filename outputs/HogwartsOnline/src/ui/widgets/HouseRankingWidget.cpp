#include "ui/widgets/HouseRankingWidget.h"

#include <QLabel>
#include <QListWidget>
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
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *title = new QLabel("House Cup", this);
    title->setObjectName("PanelTitle");

    m_list = new QListWidget(this);
    m_list->addItem("1. Ravenclaw       145");
    m_list->addItem("2. Gryffindor      132");
    m_list->addItem("3. Hufflepuff      118");
    m_list->addItem("4. Slytherin       109");

    layout->addWidget(title);
    layout->addWidget(m_list, 1);
}
