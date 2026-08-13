#pragma once

#include "application/vo/CampusVO.hpp"

#include <QString>
#include <QWidget>
#include <vector>

class QListWidget;
class QListWidgetItem;

class InventoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryWidget(QWidget *parent = nullptr);

public slots:
    // Replace the whole list with items loaded from the database.
    void refreshFromDatabase(const std::vector<arcane::application::vo::InventoryItemVO> &items);

signals:
    void itemUseRequested(const QString &itemId);
    void itemInspectRequested(const QString &itemId);
    void itemGiftRequested(const QString &itemId, const QString &npcId);

private:
    void buildUi();
    void addItem(const QString &id, const QString &name, const QString &description);
    QString itemId(QListWidgetItem *item) const;

    QListWidget *m_list = nullptr;
};
