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
    void retranslateUi();

signals:
    void memberSelected(const QString &memberId);
    void privateChatRequested(const QString &memberId);
    void viewProfileRequested(const QString &memberId);
    void npcChatRequested(const QString &npcId);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void refreshTree();
    QTreeWidgetItem *addGroup(const QString &title);
    void addMember(QTreeWidgetItem *group,
                   const QString &id,
                   const QString &name,
                   const QString &location,
                   const QString &status,
                   const QString &house);

    QLabel *m_titleLabel = nullptr;
    QTreeWidget *m_tree = nullptr;
};
