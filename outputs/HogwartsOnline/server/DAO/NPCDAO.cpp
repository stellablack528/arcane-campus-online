#include "NPCDAO.hpp"
#include <utility>

namespace arcane::database {

NPCDAO::NPCDAO(std::shared_ptr<DBConnection> connection) noexcept : connection_(std::move(connection)) {}

bool NPCDAO::createNPC(const NPCRecord& n) {
    return connection_->execute(
        "INSERT INTO npcs (npc_name, room_id, description, personality) VALUES (?, ?, ?, ?)",
        {n.npcName, std::to_string(n.roomId), n.description, n.personality});
}

std::optional<NPCRecord> NPCDAO::getNPCById(std::uint64_t id) const {
    const auto r = connection_->query("SELECT * FROM npcs WHERE npc_id = ?", {std::to_string(id)});
    static_cast<void>(r); return std::nullopt;
}

std::vector<NPCRecord> NPCDAO::getNPCsByRoom(std::uint64_t roomId) const {
    const auto r = connection_->query("SELECT * FROM npcs WHERE room_id = ?", {std::to_string(roomId)});
    static_cast<void>(r); return {};
}

bool NPCDAO::updateNPCRoom(std::uint64_t npcId, std::uint64_t roomId) {
    return connection_->execute("UPDATE npcs SET room_id = ? WHERE npc_id = ?",
                                {std::to_string(roomId), std::to_string(npcId)});
}

bool NPCDAO::deleteNPC(std::uint64_t id) {
    return connection_->execute("DELETE FROM npcs WHERE npc_id = ?", {std::to_string(id)});
}

} // namespace arcane::database
