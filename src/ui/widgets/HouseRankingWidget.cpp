#include "ui/widgets/HouseRankingWidget.h"

#include "ui/I18n.hpp"

#include <QEasingCurve>
#include <QFont>
#include <QHash>
#include <QLabel>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QStringList>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPair>

#include <algorithm>
#include <array>

namespace {

struct HouseMeta {
    const char* name;
    const char* barObject;
    const char* emoji;
    int seed;
};

// 固定展示顺序：格兰芬多、斯莱特林、拉文克劳、赫奇帕奇。
const std::array<HouseMeta, 4> kHouses = {{
    {"Gryffindor", "GryffindorBar", "\xf0\x9f\xa6\x81", 132},
    {"Slytherin",  "SlytherinBar",  "\xf0\x9f\x90\x8d", 109},
    {"Ravenclaw",  "RavenclawBar",  "\xf0\x9f\xa6\x85", 145},
    {"Hufflepuff", "HufflepuffBar", "\xf0\x9f\xa6\xa1", 118},
}};

QString rankKey(int rank)
{
    switch (rank) {
        case 1: return QStringLiteral("ranking.rank.1");
        case 2: return QStringLiteral("ranking.rank.2");
        case 3: return QStringLiteral("ranking.rank.3");
        default: return QStringLiteral("ranking.rank.4");
    }
}

} // namespace

HouseRankingWidget::HouseRankingWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("HouseRankingWidget");
    for (const auto& h : kHouses) {
        m_houseOrder << QString::fromLatin1(h.name);
        m_points[QString::fromLatin1(h.name)] = h.seed;
    }
    buildUi();
    refreshRanks();

    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) {
                if (m_titleLabel) m_titleLabel->setText(TR("title.housecup"));
                refreshRanks();
            });
}

void HouseRankingWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    auto *title = new QLabel(TR("title.housecup"), this);
    title->setObjectName("PanelTitle");
    m_titleLabel = title;

    auto *grid = new QGridLayout;
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(8);
    grid->setVerticalSpacing(10);

    int row = 0;
    for (const auto& h : kHouses) {
        const QString house = QString::fromLatin1(h.name);

        auto *rank = new QLabel(QStringLiteral("4th"), this);
        rank->setObjectName("RankBadge");
        rank->setAlignment(Qt::AlignCenter);

        auto *name = new QLabel(QString::fromUtf8(h.emoji) + QStringLiteral("  ") + house, this);
        name->setObjectName("HouseName");
        QFont nameFont = name->font();
        nameFont.setPointSize(11);
        nameFont.setWeight(QFont::DemiBold);
        name->setFont(nameFont);

        auto *bar = new QProgressBar(this);
        bar->setObjectName(QString::fromLatin1(h.barObject));
        bar->setRange(0, kMaxBar);
        bar->setAlignment(Qt::AlignCenter);
        bar->setFormat(QStringLiteral("%p%"));
        bar->setValue(std::clamp(m_points[house], 0, kMaxBar));

        auto *points = new QLabel(QString::number(m_points[house]), this);
        points->setObjectName("ValueLabel");
        points->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        auto *delta = new QLabel(QStringLiteral("\xe2\x80\x94"), this);
        delta->setObjectName("DeltaLabel");
        delta->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        grid->addWidget(rank,   row, 0);
        grid->addWidget(name,   row, 1);
        grid->addWidget(bar,    row, 2);
        grid->addWidget(points, row, 3);
        grid->addWidget(delta,  row, 4);

        grid->setColumnStretch(2, 1);

        m_rankLabels[house] = rank;
        m_bars[house] = bar;
        m_pointsLabels[house] = points;
        m_deltaLabels[house] = delta;

        ++row;
    }

    layout->addWidget(title);
    layout->addLayout(grid, 1);
}

void HouseRankingWidget::refreshRanks()
{
    // 按分数降序得出名次。
    QStringList ranked = m_houseOrder;
    std::sort(ranked.begin(), ranked.end(),
        [this](const QString &a, const QString &b) {
            return m_points[a] > m_points[b];
        });

    int rank = 1;
    for (const auto &house : ranked) {
        m_rankLabels[house]->setText(TRQ(rankKey(rank)));
        m_pointsLabels[house]->setText(QString::number(m_points[house]));
        ++rank;
    }
}

void HouseRankingWidget::animateBarTo(const QString &house, int value)
{
    const int clamped = std::clamp(value, 0, kMaxBar);
    auto *bar = m_bars[house];
    if (!bar) return;

    auto *&anim = m_anims[house];
    if (!anim) {
        anim = new QPropertyAnimation(bar, "value", this);
        anim->setDuration(380);
        anim->setEasingCurve(QEasingCurve::OutCubic);
    } else {
        anim->stop();
    }
    anim->setStartValue(bar->value());
    anim->setEndValue(clamped);
    anim->start();
}

void HouseRankingWidget::onHousePointsChanged(const QString &house, int delta, const QString &reason)
{
    if (!m_points.contains(house)) return;

    m_points[house] += delta;

    // 沙漏积分条平滑动画到新值。
    animateBarTo(house, m_points[house]);

    // 最近变化标签：带 +/- 与原因。
    const QString sign = delta >= 0 ? QStringLiteral("+") : QString();
    const QString color = delta >= 0 ? QStringLiteral("#7ec88a") : QStringLiteral("#e08a8a");
    m_deltaLabels[house]->setText(
        QString("<span style='color:%1;'>%2%3</span> \xe2\x80\x94 %4")
            .arg(color, sign, QString::number(delta), reason.toHtmlEscaped()));
    m_deltaLabels[house]->setTextFormat(Qt::RichText);

    refreshRanks();
}
