#pragma once

#include "application/narrative/NarrativeDO.hpp"
#include "application/narrative/NarrativeDTO.hpp"
#include "application/narrative/NarrativeVO.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace arcane::application::service {

// 活剧情层驱动器：每个玩家选择会对照"后果目录"解析，改动玩家的风评与
// 逐个 NPC 的关系。状态先放在内存（demo），后续可由 DAO 持久化。
class NarrativeService final {
public:
    NarrativeService() noexcept;

    // 套用一个选择，返回叙事回馈与更新后的风评。
    [[nodiscard]] vo::NarrativeResultVO applyChoice(const dto::ChoiceRequestDTO& request);

    // 只读风评快照（对新角色会先种入默认关系）。
    [[nodiscard]] vo::ReputationVO queryReputation(const dto::ReputationQueryDTO& request);

    // 全部可选选择目录（供 UI 菜单列出）。
    [[nodiscard]] const std::vector<do_model::ChoiceConsequenceDO>& availableChoices() const noexcept;

private:
    void seedCatalog();
    void seedRelationshipsFor(std::uint64_t characterId);

    std::vector<do_model::ChoiceConsequenceDO> catalog_;
    std::unordered_map<std::uint64_t, do_model::ReputationDO> reputationStore_;
};

} // namespace arcane::application::service
