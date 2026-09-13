#pragma once

#include <QString>
#include <QWidget>
#include <vector>

class QLabel;
class QTreeWidget;
class QTreeWidgetItem;

struct MemberSeed {
    QString groupKey;       // 分组 key（members.players 等）
    QString id;
    QString nameKey;        // TR key
    QString locationKey;    // TR key
    QString statusKey;      // TR key
    QString houseKey;       // TR key（house.* 或 npc.role.*）
};

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
    void retranslateUi();
    QTreeWidgetItem *addGroup(const QString &titleKey);
    void addMember(QTreeWidgetItem *group,
                   const QString &id,
                   const QString &nameKey,
                   const QString &locationKey,
                   const QString &statusKey,
                   const QString &houseKey);

    QTreeWidget *m_tree = nullptr;
    QLabel *m_titleLabel = nullptr;
    std::vector<MemberSeed> m_seed;
};