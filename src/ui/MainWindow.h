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
class LocationPanel;
class MemberListWidget;
class QComboBox;

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
    // 触发左栏"当前地点"刷新 + 此地角色列表更新。
    void updateCurrentLocationPanel(const QString &location);
    void displayMaraudersMap(const QString &spellQuote,
                             std::uint32_t onlineCount,
                             const std::vector<arcane::application::vo::MapPlayerVO> &players);
    void refreshInventory(const std::vector<arcane::application::vo::InventoryItemVO> &items);
    void onHousePointsChanged(const QString &house, int delta, const QString &reason);
    // 富文本广播通道：body 原样插入聊天面板（学院色日志/剧情回馈）。
    void appendRichCampusMessage(const QString &channel, const QString &speaker, const QString &htmlBody);
    // 开局剧情驱动：场景 Header / 输入提示 / 选项按钮。
    void setSceneHeader(const QString &title);
    void setInputHint(const QString &hint);
    void presentChoices(const QStringList &labels);

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
    void timePeriodChanged(int period);
    void nightPatrolRequested(const QString &targetId);
    // 剧情模块：做出选择 / 查询风评。
    void storyChoiceRequested(const QString &choiceId);
    void reputationQueryRequested();
    // 开局剧情：开始 / 选项点击。
    void startPrologueRequested();
    void prologueChoiceSelected(int index);

private:
    void buildMenu();
    void buildUi();
    void connectWidgetSignals();
    void retranslateUi();

    MemberListWidget *m_memberList = nullptr;
    ChatEventWidget *m_chatEvents = nullptr;
    CharacterStatusWidget *m_characterStatus = nullptr;
    InventoryWidget *m_inventory = nullptr;
    CourseScheduleWidget *m_courses = nullptr;
    CampusTimeWidget *m_campusTime = nullptr;
    HouseRankingWidget *m_houseRanking = nullptr;
    LocationActionBar *m_actionBar = nullptr;
    LocationPanel *m_locationPanel = nullptr;
    QComboBox *m_languageCombo = nullptr;
};
