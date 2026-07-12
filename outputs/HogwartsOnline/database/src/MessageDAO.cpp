#include "MessageDAO.hpp"

#include <utility>

namespace arcane::database {

MessageDAO::MessageDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

bool MessageDAO::saveMessage(const MessageRecord& message)
{
    return connection_->execute(
        "INSERT INTO chat_messages (sender_character_id, room_id, content) VALUES (?, ?, ?)",
        {std::to_string(message.senderCharacterId),
         std::to_string(message.roomId),
         message.content});
}

std::optional<MessageRecord> MessageDAO::getMessageById(std::uint64_t messageId) const
{
    const auto result = connection_->query("SELECT * FROM chat_messages WHERE message_id = ?",
                                           {std::to_string(messageId)});
    static_cast<void>(result);
    return std::nullopt;
}

std::vector<MessageRecord> MessageDAO::getRoomMessages(std::uint64_t roomId, std::uint32_t limit) const
{
    const auto result = connection_->query(
        "SELECT * FROM chat_messages WHERE room_id = ? ORDER BY send_time DESC LIMIT ?",
        {std::to_string(roomId), std::to_string(limit)});
    static_cast<void>(result);
    return {};
}

bool MessageDAO::deleteMessage(std::uint64_t messageId)
{
    return connection_->execute("DELETE FROM chat_messages WHERE message_id = ?",
                                {std::to_string(messageId)});
}

} // namespace arcane::database
