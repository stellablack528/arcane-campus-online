#pragma once

#include "application/dto/CampusDTO.hpp"
#include "application/vo/CampusVO.hpp"

#include <cstdint>
#include <memory>

namespace arcane::database {
class MessageDAO;
} // namespace arcane::database

namespace arcane::application::service {

// Validates outgoing chat and, when a MessageDAO is injected, persists messages
// to the database so conversations survive across sessions.
class ChatService final {
public:
    explicit ChatService(std::shared_ptr<database::MessageDAO> messageDao) noexcept;

    [[nodiscard]] vo::OperationResultVO validate(const dto::ChatSendRequestDTO& request) const;
    [[nodiscard]] vo::CampusMessageVO npcReply(const dto::ChatSendRequestDTO& request) const;

    // Persists a message sent by the given character. No-op (returns true) when no DAO is wired in.
    [[nodiscard]] bool saveMessage(std::uint64_t characterId,
                                   std::uint64_t roomId,
                                   const dto::ChatSendRequestDTO& request) const;

private:
    std::shared_ptr<database::MessageDAO> messageDao_;
};

} // namespace arcane::application::service
