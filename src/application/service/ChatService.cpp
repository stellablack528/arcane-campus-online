#include "application/service/ChatService.hpp"

#include "MessageDAO.hpp"

#include <utility>

namespace arcane::application::service {

ChatService::ChatService(std::shared_ptr<database::MessageDAO> messageDao) noexcept
    : messageDao_(std::move(messageDao))
{
}

vo::OperationResultVO ChatService::validate(const dto::ChatSendRequestDTO& request) const
{
    if (request.channel.empty() || request.text.empty()) {
        return {false, "A channel and message are required."};
    }
    if (request.text.size() > 500) {
        return {false, "Messages are limited to 500 characters."};
    }
    return {true, {}};
}

vo::CampusMessageVO ChatService::npcReply(const dto::ChatSendRequestDTO& request) const
{
    if (request.channel == "Location") {
        return {"Location", "Gareth Weasley", "Sounds like a fine plan. Save me a seat at breakfast."};
    }
    if (request.channel == "World") {
        return {"System", "Campus Notice", "Your message carries across the campus."};
    }
    return {"Party", "Campus Guide", "Party coordination will be available when the network session is connected."};
}

bool ChatService::saveMessage(std::uint64_t characterId,
                              std::uint64_t roomId,
                              const dto::ChatSendRequestDTO& request) const
{
    if (!messageDao_) {
        return true;
    }
    const database::MessageRecord message{0, characterId, roomId, request.text, {}};
    return messageDao_->saveMessage(message);
}

} // namespace arcane::application::service
