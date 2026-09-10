#include "application/controller/CampusController.hpp"

#include "application/dto/CampusDTO.hpp"
#include "core/DeepSeekClient.hpp"

#include <utility>

namespace arcane::application::controller {

namespace {

// Per-channel NPC persona used as the AI system prompt.
struct NpcPersona {
    const char* name;
    const char* personality;
};

NpcPersona npcForChannel(const std::string& channel)
{
    if (channel == "Location") {
        return {"Gareth Weasley",
                "Curious, experimental, warm, and easily distracted by snacks."};
    }
    if (channel == "World") {
        return {"Campus Notice",
                "A formal campus announcement voice. Concise, official, slightly theatrical."};
    }
    return {"Campus Guide",
            "A patient senior student who explains how things work at Hogwarts."};
}

} // namespace

CampusController::CampusController(QObject* parent)
    : QObject(parent)
    , sessionService_(std::make_unique<service::SessionService>(nullptr, nullptr))
    , campusService_(std::make_unique<service::CampusService>(nullptr, nullptr))
    , chatService_(std::make_unique<service::ChatService>(nullptr))
    , inventoryService_(std::make_unique<service::InventoryService>(nullptr))
    , socialService_(std::make_unique<service::SocialService>(nullptr, nullptr))
    , deepSeekClient_(std::make_unique<core::DeepSeekClient>(this))
{
    connect(deepSeekClient_.get(), &core::DeepSeekClient::replyReceived,
            this, &CampusController::onAiReplyReceived);
    connect(deepSeekClient_.get(), &core::DeepSeekClient::errorOccurred,
            this, &CampusController::onAiErrorOccurred);
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
    inventoryService_ = std::make_unique<service::InventoryService>(std::move(inventoryDao));
}

void CampusController::configureMapService(std::shared_ptr<arcane::database::InventoryDAO> inventoryDao,
                                           std::shared_ptr<arcane::database::LocationDAO> locationDao)
{
    mapService_ = std::make_unique<service::MapService>(std::move(inventoryDao),
                                                         std::move(locationDao));
}

void CampusController::configureCampusService(std::shared_ptr<arcane::database::RoomDAO> roomDao,
                                             std::shared_ptr<arcane::database::NPCDAO> npcDao)
{
    campusService_ = std::make_unique<service::CampusService>(std::move(roomDao),
                                                                std::move(npcDao));
}

void CampusController::configureSocialService(std::shared_ptr<arcane::database::FriendDAO> friendDao,
                                              std::shared_ptr<arcane::database::CharacterDAO> characterDao)
{
    socialService_ = std::make_unique<service::SocialService>(std::move(friendDao),
                                                               std::move(characterDao));
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
        (void)chatService_->saveMessage(session->characterId, 1, request);
    }

    // Prefer an AI-generated reply when the user has configured an API key.
    if (core::DeepSeekClient::hasApiKey()) {
        const auto persona = npcForChannel(request.channel);
        const QString systemPrompt =
            QStringLiteral("You are %1, a character in a Hogwarts-themed text RPG campus. "
                           "Personality: %2 Reply in 1-2 short English sentences, staying fully "
                           "in character. Never mention that you are an AI.")
                .arg(persona.name, persona.personality);
        deepSeekClient_->sendChat(systemPrompt, text,
                                  channel, QString::fromLatin1(persona.name));
        return; // The reply arrives asynchronously via onAiReplyReceived.
    }

    // Fallback: hardcoded replies when no API key is configured.
    const auto reply = chatService_->npcReply(request);
    emit campusMessageProduced(QString::fromStdString(reply.channel), QString::fromStdString(reply.speaker),
                               QString::fromStdString(reply.text));
}

void CampusController::onAiReplyReceived(const QString& channel, const QString& speaker, const QString& text)
{
    emit campusMessageProduced(channel, speaker, text);
}

void CampusController::onAiErrorOccurred(const QString& message)
{
    publish({false, message.toStdString()});
}

void CampusController::onTimePeriodChanged(int period)
{
    using namespace arcane::application::world;
    const auto timePeriod = static_cast<TimePeriod>(period);

    // 构造事件评估上下文。PlayerState 梳理：登录态/当前地点/时段。
    // 未来迁移到 TCP Server 时，服务端可以用同样的 context 在每个时段切换时评估事件。
    WorldEventContext context;
    context.period = timePeriod;
    auto* session = activeSession();
    context.playerOnline = (session != nullptr);
    context.locationId = session ? session->currentLocation : std::string{};

    // 评估世界事件并把产生的消息广播到聊天区。
    // 事件只产生"消息反馈"，不直接改数据库游戏状态（保持 AI / 事件边界清晰）。
    const auto results = evaluateWorldEvents(context);
    for (const auto& result : results) {
        emit campusMessageProduced(
            QStringLiteral("World"),
            QString::fromStdString(result.speaker),
            QString::fromStdString(result.message));
    }
}

void CampusController::handleMove(const QString& locationId)
{
    auto* session = activeSession();
    if (!session) {
        publish({false, "Enter the campus before travelling."});
        return;
    }
    const auto result = campusService_->moveTo(*session, {locationId.toStdString()});
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
    const auto result = campusService_->joinCourse(*session, {courseId.toStdString()});
    publish(result);
    if (result.success) {
        emit playerLocationChanged(QString::fromStdString(session->currentLocation),
                                   QString::fromStdString(session->currentState));
    }
}

void CampusController::handleCourseDetails(const QString& courseId)
{
    publish(campusService_->describeCourse({courseId.toStdString()}));
}

void CampusController::handleProfessorInfo(const QString& professorId)
{
    publish(campusService_->describeProfessor(professorId.toStdString()));
}

void CampusController::handleItemUse(const QString& itemId)
{
    publish(inventoryService_->useItem({itemId.toStdString(), {}}));
}

void CampusController::handleItemInspect(const QString& itemId)
{
    publish(inventoryService_->inspectItem({itemId.toStdString(), {}}));
}

void CampusController::handleItemGift(const QString& itemId, const QString& npcId)
{
    publish(inventoryService_->giftItem({itemId.toStdString(), npcId.toStdString()}));
}

void CampusController::handleStudy(const QString& locationId)
{
    publish(locationId == QStringLiteral("library")
        ? vo::OperationResultVO{true, "You settle in for a Library study session."}
        : vo::OperationResultVO{false, "Study is unavailable at this location."});
}

void CampusController::handleActivity(const QString& activityId)
{
    publish(campusService_->joinActivity(activityId.toStdString()));
}

void CampusController::handleMemberSelected(const QString& memberId)
{
    publish(memberId.isEmpty() ? vo::OperationResultVO{false, "No campus member selected."}
                               : vo::OperationResultVO{true, "Selected campus member: " + memberId.toStdString()});
}

void CampusController::handlePrivateChat(const QString& memberId)
{
    publish(socialService_->startPrivateChat(memberId.toStdString()));
}

void CampusController::handleProfile(const QString& memberId)
{
    publish(socialService_->viewProfile(memberId.toStdString()));
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
    const auto result = inventoryService_->listInventory(request);
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
