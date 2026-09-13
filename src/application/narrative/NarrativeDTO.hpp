#pragma once

#include <cstdint>
#include <string>

namespace arcane::application::dto {

// 玩家做出一个剧情选择；服务按 choiceId 查找后果。
struct ChoiceRequestDTO {
    std::uint64_t characterId = 0;
    std::string choiceId;
};

// 查询某角色当前的风评快照。
struct ReputationQueryDTO {
    std::uint64_t characterId = 0;
};

} // namespace arcane::application::dto
