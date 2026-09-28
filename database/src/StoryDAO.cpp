#include "StoryDAO.hpp"

#include "RecordMapper.hpp"

#include <cstdlib>
#include <utility>

namespace arcane::database {

StoryDAO::StoryDAO(std::shared_ptr<DBConnection> connection) noexcept
    : connection_(std::move(connection))
{
}

bool StoryDAO::upsertStoryState(const StoryStateRecord& state)
{
    return connection_ && connection_->execute(
        "INSERT INTO character_story_states (character_id, chapter_id, scene_id, stage, flags) "
        "VALUES (?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE chapter_id = VALUES(chapter_id), "
        "scene_id = VALUES(scene_id), stage = VALUES(stage), flags = VALUES(flags)",
        {std::to_string(state.characterId), state.chapterId, state.sceneId, state.stage, state.flags});
}

std::optional<StoryStateRecord> StoryDAO::getStoryState(std::uint64_t characterId) const
{
    if (!connection_) return std::nullopt;
    const auto result = connection_->query(
        "SELECT character_id, chapter_id, scene_id, stage, flags FROM character_story_states "
        "WHERE character_id = ?", {std::to_string(characterId)});
    if (!result || result->rows.empty()) return std::nullopt;
    const auto& row = result->rows.front();
    return StoryStateRecord{
        detail::integer<std::uint64_t>(row, "character_id"),
        detail::value(row, "chapter_id"),
        detail::value(row, "scene_id"),
        detail::value(row, "stage"),
        detail::value(row, "flags"),
    };
}

bool StoryDAO::upsertWand(const CharacterWandRecord& wand)
{
    return connection_ && connection_->execute(
        "INSERT INTO character_wands (character_id, wood_id, core_id, wood_name, core_name, "
        "length_inches, flexibility, handle_style, finish, color_tone, engraving, claimed) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE "
        "wood_id = VALUES(wood_id), core_id = VALUES(core_id), wood_name = VALUES(wood_name), "
        "core_name = VALUES(core_name), length_inches = VALUES(length_inches), "
        "flexibility = VALUES(flexibility), handle_style = VALUES(handle_style), "
        "finish = VALUES(finish), color_tone = VALUES(color_tone), engraving = VALUES(engraving), "
        "claimed = VALUES(claimed)",
        {std::to_string(wand.characterId), wand.woodId, wand.coreId, wand.woodName, wand.coreName,
         std::to_string(wand.lengthInches), wand.flexibility, wand.handleStyle, wand.finish,
         wand.colorTone, wand.engraving, wand.claimed ? "1" : "0"});
}

std::optional<CharacterWandRecord> StoryDAO::getWand(std::uint64_t characterId) const
{
    if (!connection_) return std::nullopt;
    const auto result = connection_->query("SELECT * FROM character_wands WHERE character_id = ?",
                                           {std::to_string(characterId)});
    if (!result || result->rows.empty()) return std::nullopt;
    const auto& row = result->rows.front();
    CharacterWandRecord wand;
    wand.characterId = detail::integer<std::uint64_t>(row, "character_id");
    wand.woodId = detail::value(row, "wood_id");
    wand.coreId = detail::value(row, "core_id");
    wand.woodName = detail::value(row, "wood_name");
    wand.coreName = detail::value(row, "core_name");
    const auto length = detail::value(row, "length_inches");
    if (!length.empty()) wand.lengthInches = std::strtod(length.c_str(), nullptr);
    wand.flexibility = detail::value(row, "flexibility");
    wand.handleStyle = detail::value(row, "handle_style");
    wand.finish = detail::value(row, "finish");
    wand.colorTone = detail::value(row, "color_tone");
    wand.engraving = detail::value(row, "engraving");
    wand.claimed = detail::boolean(row, "claimed");
    return wand;
}

bool StoryDAO::markWandClaimed(std::uint64_t characterId)
{
    return connection_ && connection_->execute(
        "UPDATE character_wands SET claimed = TRUE WHERE character_id = ?",
        {std::to_string(characterId)});
}

bool StoryDAO::upsertNpcMemory(const NpcMemoryRecord& memory)
{
    return connection_ && connection_->execute(
        "INSERT INTO npc_memories (character_id, npc_id, affinity, memory_flags) VALUES (?, ?, ?, ?) "
        "ON DUPLICATE KEY UPDATE affinity = VALUES(affinity), memory_flags = VALUES(memory_flags)",
        {std::to_string(memory.characterId), memory.npcId, std::to_string(memory.affinity),
         memory.memoryFlags});
}

std::vector<NpcMemoryRecord> StoryDAO::getNpcMemories(std::uint64_t characterId) const
{
    std::vector<NpcMemoryRecord> memories;
    if (!connection_) return memories;
    const auto result = connection_->query(
        "SELECT character_id, npc_id, affinity, memory_flags FROM npc_memories WHERE character_id = ?",
        {std::to_string(characterId)});
    if (!result) return memories;
    memories.reserve(result->rows.size());
    for (const auto& row : result->rows) {
        memories.push_back(NpcMemoryRecord{
            detail::integer<std::uint64_t>(row, "character_id"),
            detail::value(row, "npc_id"),
            detail::integer<int>(row, "affinity"),
            detail::value(row, "memory_flags"),
        });
    }
    return memories;
}

} // namespace arcane::database
