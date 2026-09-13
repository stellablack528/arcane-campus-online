#pragma once

#include <QHash>
#include <QString>
#include <QWidget>

class QLabel;
class QPushButton;
class QListWidget;

// 左侧"当前地点 + 互动选项"面板。
// - 顶部：大字显示当前地点
// - 中部：快捷动作按钮（交谈/学习/休息/活点地图/夜游）
// - 下部：移动到其他地点
// - 底部：当前地点的角色列表（可点击交谈）
class LocationPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit LocationPanel(QWidget *parent = nullptr);

public slots:
    // 根据 locationId 更新"当前地点"标题与此地角色列表。
    void setCurrentLocation(const QString &locationId);

signals:
    void moveToLocationRequested(const QString &locationId);
    void studyRequested(const QString &locationId);
    void joinActivityRequested(const QString &activityId);
    void useMaraudersMapRequested(std::uint64_t roomId);
    void nightPatrolRequested(const QString &targetId);
    void memberSelected(const QString &memberId);

private:
    void buildUi();
    void retranslateUi();
    void refreshPeopleHere();

    QLabel *m_sectionLocation = nullptr;
    QLabel *m_currentLocationName = nullptr;
    QLabel *m_sectionActions = nullptr;
    QLabel *m_sectionMove = nullptr;
    QLabel *m_sectionHere = nullptr;
    QPushButton *m_studyBtn = nullptr;
    QPushButton *m_restBtn = nullptr;
    QPushButton *m_mapBtn = nullptr;
    QPushButton *m_nightBtn = nullptr;
    QPushButton *m_talkBtn = nullptr;
    QListWidget *m_peopleList = nullptr;

    QHash<QString, QPushButton*> m_moveButtons;  // locationId -> button
    QString m_currentLocationId;                  // 当前地点 id（英文 key）
};