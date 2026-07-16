#pragma once

#include "DBConnection.hpp"

#include "../server/model/PlayerLocationDO.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace arcane::database {

/**
 * @brief DAO (Data Access Object) — 玩家位置数据访问
 *
 * 📖 DAO 的职责：
 *   只做一件事：**跟数据库打交道**
 *   - 写 SQL
 *   - 执行查询
 *   - 把结果集 → 转成 DO 返回
 *
 * ⚠️ DAO 绝对不做的：
 *   ❌ 不写业务逻辑（如：能不能用地图？）
 *   ❌ 不判断权限
 *   ❌ 不组装 VO
 */
class LocationDAO final {
public:
    explicit LocationDAO(std::shared_ptr<DBConnection> connection) noexcept;

    /**
     * @brief 查询单个玩家的当前位置
     */
    [[nodiscard]] std::optional<PlayerLocationDO> findByCharacterId(std::uint64_t characterId) const;

    /**
     * @brief 查询所有在线玩家位置（活点地图用）
     * @return 在线玩家列表
     */
    [[nodiscard]] std::vector<PlayerLocationDO> findAllOnline() const;

    /**
     * @brief 查询某个房间内的所有在线玩家
     */
    [[nodiscard]] std::vector<PlayerLocationDO> findOnlineByRoom(const std::string& roomName) const;

    /**
     * @brief 更新玩家位置
     */
    [[nodiscard]] bool updatePosition(std::uint64_t characterId,
                                      const std::string& roomName,
                                      std::int32_t xPos,
                                      std::int32_t yPos);

private:
    std::shared_ptr<DBConnection> connection_;

    /**
     * @brief 从 QueryRow 解析为 PlayerLocationDO（内部工具方法）
     */
    [[nodiscard]] static PlayerLocationDO parseRow(const QueryRow& row);
};

} // namespace arcane::database
