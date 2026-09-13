#pragma once

#include "application/service/CampusService.hpp"
#include "application/service/ChatService.hpp"
#include "application/service/InventoryService.hpp"
#include "application/service/MapService.hpp"
#include "application/service/SessionService.hpp"
#include "application/service/SocialService.hpp"
#include "application/narrative/NarrativeService.hpp"
#include "application/world/WorldClock.hpp"
#include "application/world/WorldEvent.hpp"

#include <QObject>
#include <QString>

namespace arcane::core {
class DeepSeekClient;
}

namespace arcane::application::controller {

class CampusController final : public QObject {
    Q_OBJECT

public:
    explicit CampusController(QObject* parent = nullptr);

    // Inject the database-backed DAOs. Safe to call with nullptr to run in demo mode.
    void configureSessionService(std::shared_ptr<arcane::database::UserDAO> userDao,
                                 std::shared_ptr<arcane::database::CharacterDAO> characterDao,
                                 std::shared_ptr<arcane::database::InventoryDAO> inventoryDao);
    void configureChatService(std::shared_ptr<arcane::database::MessageDAO> messageDao);
    void configureInventoryService(std::shared_ptr<arcane::database::InventoryDAO> inventoryDao);
    void configureMapService(std::shared_ptr<arcane::database::InventoryDAO> inventoryDao,
                             std::shared_ptr<arcane::database::LocationDAO> locationDao);
    void configureCampusService(std::shared_ptr<arcane::database::RoomDAO> roomDao,
                                std::shared_ptr<arcane::database::NPCDAO> npcDao);
    void configureSocialService(std::shared_ptr<arcane::database::FriendDAO> friendDao,
                                std::shared_ptr<arcane::database::CharacterDAO> characterDao);
    // 入学注册：从 EnrollmentDialog 获取请求后直接调用（非信号槽驱动）。
    void handleEnrollment(const dto::EnrollmentRequestDTO& request);

public slots:
    void handleLogin(const QString& studentName, const QString& house);
    void handleChat(const QString& channel, const QString& text);
    void handleMove(const QString& locationId);
    void handleCourseJoin(const QString& courseId);
    void handleCourseDetails(const QString& courseId);
    void handleProfessorInfo(const QString& professorId);
    void handleItemUse(const QString& itemId);
    void handleItemInspect(const QString& itemId);
    void handleItemGift(const QString& itemId, const QString& npcId);
    void handleStudy(const QString& locationId);
    void handleActivity(const QString& activityId);
    void handleMemberSelected(const QString& memberId);
    void handlePrivateChat(const QString& memberId);
    void handleProfile(const QString& memberId);
    void handleUseMaraudersMap(std::uint64_t roomId);
    void handleRefreshInventory();
    void onAiReplyReceived(const QString& channel, const QString& speaker, const QString& text);
    void onAiErrorOccurred(const QString& message);
    void onTimePeriodChanged(int period);
    // 夜游入口：targetId 为空表示独自夜游，非空表示邀请 NPC 或玩家朋友。
    void handleStartNightPatrol(const QString& targetId);
    // 学院积分手动增减槽（供 UI/剧情模块调用）。
    void slotAddHousePoints(const QString& house, int points);
    void slotDeductHousePoints(const QString& house, int points);
    // 剧情模块入口：做出选择 / 查询风评。
    void handleMakeChoice(const QString& choiceId);
    void handleQueryReputation();

signals:
    void loginAccepted(const QString& studentName, const QString& house);
    void campusMessageProduced(const QString& channel, const QString& speaker, const QString& text);
    // 富文本广播：body 原样插入 ChatEventWidget（学院色日志、剧情回馈等可信内容）。
    void richCampusMessageProduced(const QString& channel, const QString& speaker, const QString& htmlBody);
    void playerLocationChanged(const QString& location, const QString& state);
    void feedbackProduced(const QString& text);
    void maraudersMapRevealed(const QString& spellQuote,
                              std::uint32_t onlineCount,
                              const std::vector<arcane::application::vo::MapPlayerVO>& players);
    void inventoryRefreshed(const std::vector<arcane::application::vo::InventoryItemVO>& items);
    // 学院积分变化（delta 正=加分，负=扣分）。
    void housePointsChanged(const QString& house, int delta, const QString& reason);

private:
    [[nodiscard]] do_model::PlayerSessionDO* activeSession();
    void publish(const vo::OperationResultVO& result);
    // 统一的学院积分变化出口：emit housePointsChanged 信号。
    void applyHousePointsChange(const std::string& house, int delta, const std::string& reason);

    std::unique_ptr<service::SessionService> sessionService_;
    std::unique_ptr<service::CampusService> campusService_;
    std::unique_ptr<service::ChatService> chatService_;
    std::unique_ptr<service::InventoryService> inventoryService_;
    std::unique_ptr<service::SocialService> socialService_;
    std::unique_ptr<service::MapService> mapService_;
    std::unique_ptr<service::NarrativeService> narrativeService_;
    std::unique_ptr<core::DeepSeekClient> deepSeekClient_;
};

} // namespace arcane::application::controller
