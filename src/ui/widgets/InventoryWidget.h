#pragma once

#include "application/vo/CampusVO.hpp"

#include <QString>
#include <QWidget>
#include <vector>

class QLabel;
class QListWidget;
class QListWidgetItem;

struct ItemSeed {
    QString id;
    QString nameKey;
    QString description;
};

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
    void addItem(const QString &id, const QString &nameKey, const QString &description);
    QString itemId(QListWidgetItem *item) const;

    QLabel *m_titleLabel = nullptr;
    QListWidget *m_list = nullptr;
    std::vector<ItemSeed> m_seed;
};