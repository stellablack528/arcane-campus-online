#pragma once

#include <QWidget>

class QLabel;
class QListWidget;

class HouseRankingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HouseRankingWidget(QWidget *parent = nullptr);
    void retranslateUi();

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void refreshRanking();

    QLabel *m_titleLabel = nullptr;
    QListWidget *m_list = nullptr;
};
