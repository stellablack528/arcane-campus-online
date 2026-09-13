#include "ui/widgets/HouseRankingWidget.h"

#include <QFont>
#include <QHash>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStringList>
#include <QVBoxLayout>

#include <algorithm>

HouseRankingWidget::HouseRankingWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("HouseRankingWidget");
    // 初始分数（与种子数据一致）。
    m_points["Ravenclaw"] = 145;
    m_points["Gryffindor"] = 132;
    m_points["Hufflepuff"] = 118;
    m_points["Slytherin"] = 109;
    buildUi();
    refreshList();
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

    m_recentChangeLabel = new QLabel("No recent point changes.", this);
    m_recentChangeLabel->setWordWrap(true);
    m_recentChangeLabel->setObjectName("ValueLabel");

    layout->addWidget(title);
    layout->addWidget(m_list, 1);
    layout->addWidget(m_recentChangeLabel);
}

void HouseRankingWidget::refreshList()
{
    m_list->clear();

    static const QHash<QString, QString> emojis = {
        {"Ravenclaw",   QString::fromUtf8("\xf0\x9f\xa6\x85")},
        {"Gryffindor",  QString::fromUtf8("\xf0\x9f\xa6\x81")},
        {"Hufflepuff",  QString::fromUtf8("\xf0\x9f\xa6\xa1")},
        {"Slytherin",   QString::fromUtf8("\xf0\x9f\x90\x8d")},
    };

    // 按分数降序排列
    QStringList houses = m_points.keys();
    std::sort(houses.begin(), houses.end(),
              [this](const QString &a, const QString &b) {
                  return m_points[a] > m_points[b];
              });

    int rank = 1;
    for (const auto &house : houses) {
        const int pts = m_points[house];
        const QString emoji = emojis.value(house, QStringLiteral("\xe2\x9c\xa8"));
        auto *item = new QListWidgetItem(
            QString("%1. %2  %3  \xe2\x80\x94  %4 pts").arg(rank).arg(emoji).arg(house).arg(pts, 3),
            m_list);
        QFont font = item->font();
        font.setPointSize(12);
        item->setFont(font);
        item->setSizeHint(QSize(0, 32));
        ++rank;
    }
}

void HouseRankingWidget::onHousePointsChanged(const QString &house, int delta, const QString &reason)
{
    // 更新分数
    if (m_points.contains(house)) {
        m_points[house] += delta;
    }

    // 更新最近变化 label（显示"学院 +X/-X pts — 原因"）
    const QString sign = delta >= 0 ? QStringLiteral("+") : QString();
    m_recentChangeLabel->setText(
        QString("%1 %2%3 pts \xe2\x80\x94 %4")
            .arg(house)
            .arg(sign)
            .arg(delta)
            .arg(reason));

    refreshList();
}
