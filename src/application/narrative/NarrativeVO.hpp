#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace arcane::application::vo {

// 单条关系，扁平化用于展示。
struct RelationshipVO {
    std::string npcId;
    std::string displayName;
    std::string role;
    std::int16_t favorability = 0;
    std::int16_t trust = 0;
    std::string standing;          // 派生标签："Trusted friend"、"Rival"...
    std::vector<std::string> tags;
};

// 返回给 UI 的风评快照。
struct ReputationVO {
    bool success = false;
    std::string message;
    std::int16_t peerReputation = 0;
    std::int16_t teacherReputation = 0;
    std::int16_t houseReputation = 0;
    std::string peerStanding;      // "Beloved" / "Liked" / "Disliked" ...
    std::string teacherStanding;
    std::string houseStanding;
    std::vector<RelationshipVO> relationships;
};

// 套用一个选择后的结果：叙事回馈 + 更新后的风评。
struct NarrativeResultVO {
    bool success = false;
    std::string message;            // 叙事文本
    std::string consequenceSummary; // 人类可读的变动摘要
    ReputationVO reputation;        // 更新后的风评
};

} // namespace arcane::application::vo
