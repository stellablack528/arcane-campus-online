#pragma once

#include <QMainWindow>
#include <QString>

class CampusTimeWidget;
class CharacterStatusWidget;
class ChatEventWidget;
class CourseScheduleWidget;
class HouseRankingWidget;
class InventoryWidget;
class LocationActionBar;
class MemberListWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void setPlayerIdentity(const QString &studentName, const QString &houseName);

signals:
    void sendChatRequested(const QString &channel, const QString &text);
    void moveToLocationRequested(const QString &locationId);
    void joinClassRequested(const QString &courseId);
    void itemUseRequested(const QString &itemId);
    void itemInspectRequested(const QString &itemId);
    void itemGiftRequested(const QString &itemId, const QString &npcId);

private:
    void buildMenu();
    void buildUi();
    void connectWidgetSignals();

    MemberListWidget *m_memberList = nullptr;
    ChatEventWidget *m_chatEvents = nullptr;
    CharacterStatusWidget *m_characterStatus = nullptr;
    InventoryWidget *m_inventory = nullptr;
    CourseScheduleWidget *m_courses = nullptr;
    CampusTimeWidget *m_campusTime = nullptr;
    HouseRankingWidget *m_houseRanking = nullptr;
    LocationActionBar *m_actionBar = nullptr;
};
