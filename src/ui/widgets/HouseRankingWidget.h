#pragma once

#include <QWidget>

class QListWidget;

class HouseRankingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HouseRankingWidget(QWidget *parent = nullptr);

private:
    void buildUi();

    QListWidget *m_list = nullptr;
};
