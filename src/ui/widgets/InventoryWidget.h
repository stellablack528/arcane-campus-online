#pragma once

#include "application/vo/CampusVO.hpp"

#include <QString>
#include <QWidget>
#include <vector>

class QLabel;
class QTreeWidget;
class QTreeWidgetItem;

class InventoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryWidget(QWidget *parent = nullptr);

public slots:
    void refreshFromDatabase(const std::vector<arcane::application::vo::InventoryItemVO> &items);

signals:
    void itemUseRequested(const QString &itemId);
    void itemInspectRequested(const QString &itemId);
    void itemGiftRequested(const QString &itemId, const QString &npcId);

private:
    void buildUi();
    void retranslateUi();
    QString itemId(QTreeWidgetItem *item) const;

    QLabel *m_titleLabel = nullptr;
    QTreeWidget *m_list = nullptr;
    std::vector<arcane::application::vo::InventoryItemVO> m_items;
};
