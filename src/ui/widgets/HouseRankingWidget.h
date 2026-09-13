#pragma once

#include <QHash>
#include <QString>
#include <QWidget>

class QLabel;
class QListWidget;

class HouseRankingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HouseRankingWidget(QWidget *parent = nullptr);

public slots:
    // 接收学院积分变化，更新排行和"最近变化"label。
    void onHousePointsChanged(const QString &house, int delta, const QString &reason);

private:
    void buildUi();
    void refreshList();

    QListWidget *m_list = nullptr;
    QLabel *m_recentChangeLabel = nullptr;
    QHash<QString, int> m_points;  // 学院名 → 当前分数
};
