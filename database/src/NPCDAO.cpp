#include "NPCDAO.hpp"
#include "RecordMapper.hpp"

#include <utility>

namespace arcane::database {

NPCDAO::NPCDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

bool NPCDAO::createNPC(const NPCRecord& npc)
{
    return connection_->execute(
        "INSERT INTO npcs (npc_name, room_id, description, personality) VALUES (?, ?, ?, ?)",
        {npc.npcName, std::to_string(npc.roomId), npc.description, npc.personality});
}

std::optional<NPCRecord> NPCDAO::getNPCById(std::uint64_t npcId) const
{
    const auto result = connection_->query("SELECT * FROM npcs WHERE npc_id = ?", {std::to_string(npcId)});
    if (!result || result->rows.empty()) {
        return std::nullopt;
    }
    const auto& row = result->rows.front();
    return NPCRecord{detail::integer<std::uint64_t>(row, "npc_id"), detail::value(row, "npc_name"),
                     detail::integer<std::uint64_t>(row, "room_id"), detail::value(row, "description"),
                     detail::value(row, "personality")};
}

std::vector<NPCRecord> NPCDAO::getNPCsByRoom(std::uint64_t roomId) const
{
    const auto result = connection_->query("SELECT * FROM npcs WHERE room_id = ?",
                                           {std::to_string(roomId)});
    std::vector<NPCRecord> npcs;
    if (!result) {
        return npcs;
    }
    npcs.reserve(result->rows.size());
    for (const auto& row : result->rows) {
        npcs.push_back(NPCRecord{detail::integer<std::uint64_t>(row, "npc_id"),
                                 detail::value(row, "npc_name"), detail::integer<std::uint64_t>(row, "room_id"),
                                 detail::value(row, "description"), detail::value(row, "personality")});
    }
    return npcs;
}

bool NPCDAO::updateNPCRoom(std::uint64_t npcId, std::uint64_t roomId)
{
    return connection_->execute("UPDATE npcs SET room_id = ? WHERE npc_id = ?",
                                {std::to_string(roomId), std::to_string(npcId)});
}

bool NPCDAO::deleteNPC(std::uint64_t npcId)
{
    return connection_->execute("DELETE FROM npcs WHERE npc_id = ?", {std::to_string(npcId)});
}

} // namespace arcane::database
