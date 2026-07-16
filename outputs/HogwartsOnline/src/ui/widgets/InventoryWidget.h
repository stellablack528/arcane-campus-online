#pragma once

#include <QString>
#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;

class InventoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryWidget(QWidget *parent = nullptr);
    void retranslateUi();

signals:
    void itemUseRequested(const QString &itemId);
    void itemInspectRequested(const QString &itemId);
    void itemGiftRequested(const QString &itemId, const QString &npcId);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void refreshItems();
    void addItem(const QString &id, const QString &name, const QString &description);
    QString itemId(QListWidgetItem *item) const;

    QLabel *m_titleLabel = nullptr;
    QListWidget *m_list = nullptr;
};
