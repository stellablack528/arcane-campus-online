#pragma once

#include "application/service/CampusService.hpp"
#include "application/service/ChatService.hpp"
#include "application/service/InventoryService.hpp"
#include "application/service/MapService.hpp"
#include "application/service/SessionService.hpp"
#include "application/service/SocialService.hpp"

#include <QObject>
#include <QString>

namespace arcane::application::controller {

class CampusController final : public QObject {
    Q_OBJECT

public:
    explicit CampusController(QObject* parent = nullptr);

    // Inject the database-backed DAOs. Safe to call with nullptr to run in demo mode.
    void configureSessionService(std::shared_ptr<arcane::database::UserDAO> userDao,
                                 std::shared_ptr<arcane::database::CharacterDAO> characterDao);
    void configureChatService(std::shared_ptr<arcane::database::MessageDAO> messageDao);
    void configureInventoryService(std::shared_ptr<arcane::database::InventoryDAO> inventoryDao);
    void configureMapService(std::shared_ptr<arcane::database::InventoryDAO> inventoryDao,
                             std::shared_ptr<arcane::database::LocationDAO> locationDao);

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

signals:
    void loginAccepted(const QString& studentName, const QString& house);
    void campusMessageProduced(const QString& channel, const QString& speaker, const QString& text);
    void playerLocationChanged(const QString& location, const QString& state);
    void feedbackProduced(const QString& text);
    void maraudersMapRevealed(const QString& spellQuote,
                              std::uint32_t onlineCount,
                              const std::vector<arcane::application::vo::MapPlayerVO>& players);
    void inventoryRefreshed(const std::vector<arcane::application::vo::InventoryItemVO>& items);

private:
    [[nodiscard]] do_model::PlayerSessionDO* activeSession();
    void publish(const vo::OperationResultVO& result);

    std::unique_ptr<service::SessionService> sessionService_;
    service::CampusService campusService_;
    std::unique_ptr<service::ChatService> chatService_;
    service::InventoryService inventoryService_;
    service::SocialService socialService_;
    std::unique_ptr<service::MapService> mapService_;
};

} // namespace arcane::application::controller
