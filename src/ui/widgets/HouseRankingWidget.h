#pragma once

#include <QHash>
#include <QString>
#include <QWidget>

class QLabel;
class QProgressBar;
class QPropertyAnimation;

class HouseRankingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HouseRankingWidget(QWidget *parent = nullptr);

public slots:
    // 接收学院积分变化，更新沙漏积分条与排行徽章。
    void onHousePointsChanged(const QString &house, int delta, const QString &reason);

private:
    void buildUi();
    void refreshRanks();
    void animateBarTo(const QString &house, int value);

    // 沙漏条的最大刻度（学院分在此区间内按比例填充）。
    static constexpr int kMaxBar = 300;

    QStringList m_houseOrder;                  // 固定展示顺序
    QHash<QString, QProgressBar*> m_bars;       // 学院 → 积分条
    QHash<QString, QLabel*> m_rankLabels;       // 学院 → 名次徽章
    QHash<QString, QLabel*> m_pointsLabels;     // 学院 → 分数
    QHash<QString, QLabel*> m_deltaLabels;      // 学院 → 最近变化
    QHash<QString, QPropertyAnimation*> m_anims;// 学院 → 动画
    QHash<QString, int> m_points;               // 学院 → 当前分数
};
