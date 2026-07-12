#pragma once

#include <QString>
#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;

class MemberListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MemberListWidget(QWidget *parent = nullptr);

signals:
    void memberSelected(const QString &memberId);
    void privateChatRequested(const QString &memberId);
    void viewProfileRequested(const QString &memberId);

private:
    void buildUi();
    QTreeWidgetItem *addGroup(const QString &title);
    void addMember(QTreeWidgetItem *group,
                   const QString &id,
                   const QString &name,
                   const QString &location,
                   const QString &status,
                   const QString &house);

    QTreeWidget *m_tree = nullptr;
};
