#pragma once

#include <QMainWindow>
#include <QString>

class CampusTimeWidget;
class CharacterStatusWidget;
class ChatEventWidget;
class CourseScheduleWidget;
class HouseRankingWidget;
class InventoryWidget;
class LocationBrowserWidget;
class DeepSeekClient;
class MemberListWidget;
class QActionGroup;
class QMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void retranslateUi();

public slots:
    void setPlayerIdentity(const QString &studentName, const QString &houseName);
    void setWandInfo(const QString &wood, const QString &core, int lengthInches,
                     const QString &flexibility);

signals:
    void sendChatRequested(const QString &channel, const QString &text);
    void moveToLocationRequested(const QString &locationId);
    void joinClassRequested(const QString &courseId);
    void itemUseRequested(const QString &itemId);
    void itemInspectRequested(const QString &itemId);
    void itemGiftRequested(const QString &itemId, const QString &npcId);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onNpcChatRequested(const QString &npcId);

private:
    void buildMenu();
    void buildUi();
    void connectWidgetSignals();

    DeepSeekClient *m_deepseek = nullptr;
    QMenu *m_characterMenu = nullptr;
    QMenu *m_campusMenu = nullptr;
    QMenu *m_socialMenu = nullptr;
    QMenu *m_systemMenu = nullptr;
    QMenu *m_languageMenu = nullptr;
    QActionGroup *m_langGroup = nullptr;

    MemberListWidget *m_memberList = nullptr;
    ChatEventWidget *m_chatEvents = nullptr;
    CharacterStatusWidget *m_characterStatus = nullptr;
    InventoryWidget *m_inventory = nullptr;
    CourseScheduleWidget *m_courses = nullptr;
    CampusTimeWidget *m_campusTime = nullptr;
    HouseRankingWidget *m_houseRanking = nullptr;
    LocationBrowserWidget *m_locationBrowser = nullptr;
};
