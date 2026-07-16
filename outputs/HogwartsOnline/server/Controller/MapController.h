#pragma once

#include "Service/MapService.h"
#include "Query/MapUseQuery.h"
#include "VO/MapResponseVO.h"

#include <cstdint>
#include <memory>

namespace arcane::controller {

/**
 * Controller — 活点地图接口
 *
 * 📖 职责：接收请求 → 解析参数 → 调用 Service → 返回 VO
 *
 * 🔗 GET /map/use-marauders-map?characterId={id}
 */
class MapController {
public:
    explicit MapController(std::shared_ptr<arcane::service::MapService> service);

    [[nodiscard]] vo::MapResponseVO useMaraudersMap(std::uint64_t characterId) const;

private:
    std::shared_ptr<arcane::service::MapService> service_;
};

} // namespace arcane::controller
