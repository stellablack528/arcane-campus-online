#pragma once

#include "application/vo/CampusVO.hpp"

#include <QMainWindow>
#include <QString>
#include <vector>

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
    void appendCampusMessage(const QString &channel, const QString &speaker, const QString &text);
    void updatePlayerLocation(const QString &location, const QString &state);
    void showFeedback(const QString &text);
    void displayMaraudersMap(const QString &spellQuote,
                             std::uint32_t onlineCount,
                             const std::vector<arcane::application::vo::MapPlayerVO> &players);
    void refreshInventory(const std::vector<arcane::application::vo::InventoryItemVO> &items);

signals:
    void sendChatRequested(const QString &channel, const QString &text);
    void moveToLocationRequested(const QString &locationId);
    void joinClassRequested(const QString &courseId);
    void itemUseRequested(const QString &itemId);
    void itemInspectRequested(const QString &itemId);
    void itemGiftRequested(const QString &itemId, const QString &npcId);
    void courseDetailsRequested(const QString &courseId);
    void professorInfoRequested(const QString &professorId);
    void studyRequested(const QString &locationId);
    void activityJoinRequested(const QString &activityId);
    void memberSelected(const QString &memberId);
    void privateChatRequested(const QString &memberId);
    void profileRequested(const QString &memberId);
    void useMaraudersMapRequested(std::uint64_t roomId);
    void refreshInventoryRequested();

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
