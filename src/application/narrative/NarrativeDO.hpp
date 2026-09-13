#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace arcane::application::do_model {

// ---------------------------------------------------------------------------
// 剧情模块数据模型
//
// 设计目标：让玩家"作为霍格沃兹的一个学生，做的每个选择都有真实影响"。
// 每次选择都会改动 ReputationDO / RelationshipDO，并留下可追溯的痕迹。
// ---------------------------------------------------------------------------

// 玩家与某个校园角色（同学/老师/室友/对手/教职工）之间的一条关系弧线。
// 选择会持续改动这些字段，使每个决定都留下有形的痕迹。
struct RelationshipDO {
    std::string npcId;               // 稳定 id，如 "hermione"、"snape"
    std::string displayName;         // 展示名，如 "Hermione Granger"
    std::string role;                // classmate | teacher | housemate | rival | staff
    std::int16_t favorability = 0;   // 好感度 -100..100
    std::int16_t trust = 0;          // 信任度 -100..100
    std::vector<std::string> tags;   // "trusted","rival","crush","mentor","helpful"
    std::string note;                // 最近一次印象备注
};

// 玩家角色的聚合风评快照。
struct ReputationDO {
    std::uint64_t characterId = 0;
    std::int16_t peerReputation = 0;     // 同学整体风评 -100..100
    std::int16_t teacherReputation = 0;  // 老师整体看法 -100..100
    std::int16_t houseReputation = 0;    // 本学院内声望 -100..100
    std::vector<RelationshipDO> relationships;
};

// 描述一个玩家选择的后果。NarrativeService 持有一份目录并在调用时套用。
struct ChoiceConsequenceDO {
    std::string choiceId;          // "share_notes_with_hermione"
    std::string label;             // 简短 UI 标签
    std::string description;       // 这个选择是什么
    std::int16_t peerDelta = 0;    // 同学风评变动
    std::int16_t teacherDelta = 0; // 老师看法变动
    std::int16_t houseDelta = 0;   // 学院声望变动
    std::string targetNpcId;       // 受影响的关系目标（可空）
    std::int16_t favorabilityDelta = 0;
    std::int16_t trustDelta = 0;
    std::vector<std::string> addTags;  // 追加到目标关系的标签
    std::string narrativeText;     // 回馈玩家的叙事文本
};

} // namespace arcane::application::do_model
