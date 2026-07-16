#pragma once

#include "../database/include/DBConnection.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace arcane::database {

struct NPCRecord {
    std::uint64_t npcId = 0;
    std::string npcName;
    std::uint64_t roomId = 0;
    std::string description;
    std::string personality;
};

class NPCDAO final {
public:
    explicit NPCDAO(std::shared_ptr<DBConnection> connection) noexcept;
    [[nodiscard]] bool createNPC(const NPCRecord& npc);
    [[nodiscard]] std::optional<NPCRecord> getNPCById(std::uint64_t id) const;
    [[nodiscard]] std::vector<NPCRecord> getNPCsByRoom(std::uint64_t roomId) const;
    [[nodiscard]] bool updateNPCRoom(std::uint64_t npcId, std::uint64_t roomId);
    [[nodiscard]] bool deleteNPC(std::uint64_t id);
private:
    std::shared_ptr<DBConnection> connection_;
};

} // namespace arcane::database
