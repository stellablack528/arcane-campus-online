#include "MapController.h"

namespace arcane::controller {

MapController::MapController(std::shared_ptr<arcane::service::MapService> service)
    : service_(std::move(service)) {}

vo::MapResponseVO MapController::useMaraudersMap(std::uint64_t characterId) const {
    return service_->useMaraudersMap(characterId);
}

} // namespace arcane::controller
