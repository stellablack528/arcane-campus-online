#pragma once

#include <cstdint>
#include <string>

namespace arcane::application::dto {

struct LoginRequestDTO {
    std::string studentName;
    std::string house;
};

struct ChatSendRequestDTO {
    std::string channel;
    std::string text;
};

struct LocationMoveRequestDTO {
    std::string locationId;
};

struct CourseActionRequestDTO {
    std::string courseId;
};

struct ItemActionRequestDTO {
    std::string itemId;
    std::string npcId;
};

struct QuestProgressRequestDTO {
    std::uint64_t characterId = 0;
    std::uint64_t questId = 0;
    std::uint32_t progress = 0;
    std::string status;
};

// Request body for "use the Marauder's Map". roomId == 0 means "reveal the whole campus".
struct MapUseRequestDTO {
    std::uint64_t characterId = 0;
    std::uint64_t roomId = 0;
};

// Request body for listing a character's inventory (used by the refreshed InventoryService).
struct InventoryListRequestDTO {
    std::uint64_t characterId = 0;
};

// 夜游请求：可邀请 NPC 或玩家朋友（targetId 为空表示独自夜游）。
struct NightPatrolRequestDTO {
    std::string targetId;
};

// 入学请求：角色信息 + 预先登记的魔杖参数。魔杖与动物都不会在
// 创建界面直接发放；实际获得时机由对角巷剧情决定。
struct EnrollmentRequestDTO {
    // 角色基本信息
    std::string username;      // 登录用户名
    std::string nickname;      // 角色昵称
    std::string house;         // 学院
    std::string gender;        // 性别 (male/female/other)
    std::string hairColor;     // 发色
    std::string eyeColor;      // 瞳色
    std::string bloodStatus;   // 血统 (pureblood/halfblood/muggleborn)
    // 魔杖选择（基于 wand_customization.json）
    std::string wandWoodId;    // 木材 id (如 "holly")
    std::string wandCoreId;    // 核心 id (如 "phoenix_feather")
    std::string wandWoodName;  // 当前语言下用于剧情展示的名称
    std::string wandCoreName;
    double wandLength = 11.0;  // 长度 (9.0-14.0)
    std::string wandFlexibility;
    std::string wandHandleStyle;
    std::string wandFinish;
    std::string wandColorTone;
    std::string wandEngraving; // 可选刻字
    std::string wandName;      // 可选自定义魔杖名
    std::string wandDescription;
    std::string petType = "none"; // 宠物留给后续商店剧情
};

} // namespace arcane::application::dto
