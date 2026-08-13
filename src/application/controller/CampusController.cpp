#include "application/controller/CampusController.hpp"

#include "application/dto/CampusDTO.hpp"

#include <utility>

namespace arcane::application::controller {

CampusController::CampusController(QObject* parent)
    : QObject(parent)
    , sessionService_(std::make_unique<service::SessionService>(nullptr, nullptr))
    , chatService_(std::make_unique<service::ChatService>(nullptr))
{
}

void CampusController::configureSessionService(std::shared_ptr<arcane::database::UserDAO> userDao,
                                               std::shared_ptr<arcane::database::CharacterDAO> characterDao)
{
    sessionService_ = std::make_unique<service::SessionService>(std::move(userDao),
                                                                 std::move(characterDao));
}

void CampusController::configureChatService(std::shared_ptr<arcane::database::MessageDAO> messageDao)
{
    chatService_ = std::make_unique<service::ChatService>(std::move(messageDao));
}

void CampusController::configureInventoryService(std::shared_ptr<arcane::database::InventoryDAO> inventoryDao)
{
    inventoryService_ = service::InventoryService(std::move(inventoryDao));
}

void CampusController::configureMapService(std::shared_ptr<arcane::database::InventoryDAO> inventoryDao,
                                           std::shared_ptr<arcane::database::LocationDAO> locationDao)
{
    mapService_ = std::make_unique<service::MapService>(std::move(inventoryDao),
                                                         std::move(locationDao));
}

void CampusController::handleLogin(const QString& studentName, const QString& house)
{
    const auto result = sessionService_->enterCampus({studentName.toStdString(), house.toStdString()});
    if (!result.success) {
        publish({false, result.message});
        return;
    }
    emit loginAccepted(QString::fromStdString(result.studentName), QString::fromStdString(result.house));
    emit playerLocationChanged(QString::fromStdString(result.location), QStringLiteral("Breakfast"));
    emit campusMessageProduced(QStringLiteral("System"), QStringLiteral("Campus Notice"),
                               QString::fromStdString(result.message));
}

void CampusController::handleChat(const QString& channel, const QString& text)
{
    const dto::ChatSendRequestDTO request{channel.toStdString(), text.trimmed().toStdString()};
    const auto validation = chatService_->validate(request);
    if (!validation.success) {
        publish(validation);
        return;
    }

    // Persist the message when a database session is active. The current single-room
    // demo defaults to the Great Hall (room_id 1) until multi-room routing lands.
    if (auto* session = activeSession()) {
        chatService_->saveMessage(session->characterId, 1, request);
    }

    const auto reply = chatService_->npcReply(request);
    emit campusMessageProduced(QString::fromStdString(reply.channel), QString::fromStdString(reply.speaker),
                               QString::fromStdString(reply.text));
}

void CampusController::handleMove(const QString& locationId)
{
    auto* session = activeSession();
    if (!session) {
        publish({false, "Enter the campus before travelling."});
        return;
    }
    const auto result = campusService_.moveTo(*session, {locationId.toStdString()});
    publish(result);
    if (result.success) {
        emit playerLocationChanged(QString::fromStdString(session->currentLocation),
                                   QString::fromStdString(session->currentState));
    }
}

void CampusController::handleCourseJoin(const QString& courseId)
{
    auto* session = activeSession();
    if (!session) {
        publish({false, "Enter the campus before joining a class."});
        return;
    }
    const auto result = campusService_.joinCourse(*session, {courseId.toStdString()});
    publish(result);
    if (result.success) {
        emit playerLocationChanged(QString::fromStdString(session->currentLocation),
                                   QString::fromStdString(session->currentState));
    }
}

void CampusController::handleCourseDetails(const QString& courseId)
{
    publish(campusService_.describeCourse({courseId.toStdString()}));
}

void CampusController::handleProfessorInfo(const QString& professorId)
{
    publish(campusService_.describeProfessor(professorId.toStdString()));
}

void CampusController::handleItemUse(const QString& itemId)
{
    publish(inventoryService_.useItem({itemId.toStdString(), {}}));
}

void CampusController::handleItemInspect(const QString& itemId)
{
    publish(inventoryService_.inspectItem({itemId.toStdString(), {}}));
}

void CampusController::handleItemGift(const QString& itemId, const QString& npcId)
{
    publish(inventoryService_.giftItem({itemId.toStdString(), npcId.toStdString()}));
}

void CampusController::handleStudy(const QString& locationId)
{
    publish(locationId == QStringLiteral("library")
        ? vo::OperationResultVO{true, "You settle in for a Library study session."}
        : vo::OperationResultVO{false, "Study is unavailable at this location."});
}

void CampusController::handleActivity(const QString& activityId)
{
    publish(campusService_.joinActivity(activityId.toStdString()));
}

void CampusController::handleMemberSelected(const QString& memberId)
{
    publish(memberId.isEmpty() ? vo::OperationResultVO{false, "No campus member selected."}
                               : vo::OperationResultVO{true, "Selected campus member: " + memberId.toStdString()});
}

void CampusController::handlePrivateChat(const QString& memberId)
{
    publish(socialService_.startPrivateChat(memberId.toStdString()));
}

void CampusController::handleProfile(const QString& memberId)
{
    publish(socialService_.viewProfile(memberId.toStdString()));
}

void CampusController::handleUseMaraudersMap(std::uint64_t roomId)
{
    if (!mapService_) {
        publish({false, "The Marauder's Map is not available in demo mode."});
        return;
    }
    auto* session = activeSession();
    if (!session) {
        publish({false, "Enter the campus before consulting the map."});
        return;
    }

    const dto::MapUseRequestDTO request{session->characterId, roomId};
    const auto result = mapService_->useMaraudersMap(request);

    if (!result.success) {
        publish({false, result.message});
        return;
    }

    emit maraudersMapRevealed(QString::fromStdString(result.spellQuote),
                              result.onlineCount,
                              result.players);
    publish({true, result.message});
}

void CampusController::handleRefreshInventory()
{
    auto* session = activeSession();
    if (!session) {
        publish({false, "Enter the campus before opening your inventory."});
        return;
    }
    const dto::InventoryListRequestDTO request{session->characterId};
    const auto result = inventoryService_.listInventory(request);
    if (!result.success) {
        publish({false, result.message});
        return;
    }
    emit inventoryRefreshed(result.items);
    publish({true, result.message});
}

do_model::PlayerSessionDO* CampusController::activeSession()
{
    return sessionService_ ? sessionService_->activeSession() : nullptr;
}

void CampusController::publish(const vo::OperationResultVO& result)
{
    emit feedbackProduced(QString::fromStdString(result.message));
}

} // namespace arcane::application::controller
