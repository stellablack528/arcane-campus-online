#include "MessageDAO.hpp"
#include "RecordMapper.hpp"

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
    if (!result || result->rows.empty()) {
        return std::nullopt;
    }
    const auto& row = result->rows.front();
    return MessageRecord{detail::integer<std::uint64_t>(row, "message_id"),
                         detail::integer<std::uint64_t>(row, "sender_character_id"),
                         detail::integer<std::uint64_t>(row, "room_id"), detail::value(row, "content"),
                         detail::value(row, "send_time")};
}

std::vector<MessageRecord> MessageDAO::getRoomMessages(std::uint64_t roomId, std::uint32_t limit) const
{
    const auto result = connection_->query(
        "SELECT * FROM chat_messages WHERE room_id = ? ORDER BY send_time DESC LIMIT "
            + std::to_string(limit),
        {std::to_string(roomId)});
    std::vector<MessageRecord> messages;
    if (!result) {
        return messages;
    }
    messages.reserve(result->rows.size());
    for (const auto& row : result->rows) {
        messages.push_back(MessageRecord{detail::integer<std::uint64_t>(row, "message_id"),
                                         detail::integer<std::uint64_t>(row, "sender_character_id"),
                                         detail::integer<std::uint64_t>(row, "room_id"),
                                         detail::value(row, "content"), detail::value(row, "send_time")});
    }
    return messages;
}

bool MessageDAO::updateMessageContent(std::uint64_t messageId, const std::string& content)
{
    return connection_->execute("UPDATE chat_messages SET content = ? WHERE message_id = ?",
                                {content, std::to_string(messageId)});
}

bool MessageDAO::deleteMessage(std::uint64_t messageId)
{
    return connection_->execute("DELETE FROM chat_messages WHERE message_id = ?",
                                {std::to_string(messageId)});
}

} // namespace arcane::database
