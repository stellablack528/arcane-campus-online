#pragma once

#include "DBConnection.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace arcane::database {

struct MessageRecord {
    std::uint64_t messageId = 0;
    std::uint64_t senderCharacterId = 0;
    std::uint64_t roomId = 0;
    std::string content;
    std::string sendTime;
};

class MessageDAO final {
public:
    explicit MessageDAO(std::shared_ptr<DBConnection> connection) noexcept;

    [[nodiscard]] bool saveMessage(const MessageRecord& message);
    [[nodiscard]] std::optional<MessageRecord> getMessageById(std::uint64_t messageId) const;
    [[nodiscard]] std::vector<MessageRecord> getRoomMessages(std::uint64_t roomId, std::uint32_t limit) const;
    [[nodiscard]] bool deleteMessage(std::uint64_t messageId);

private:
    std::shared_ptr<DBConnection> connection_;
};

} // namespace arcane::database
