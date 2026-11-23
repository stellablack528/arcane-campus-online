#pragma once

#include "application/open_world/npc/NpcCatalog.hpp"

#include <cstdint>
#include <memory>

namespace arcane::database { class DBConnection; }

namespace arcane::application::open_world::npc {

struct TalkContext {
    std::uint64_t characterId = 0;
    QString era;
    QString scene;
    int phase = 0;
    bool chinese = true;
};

struct TalkResult {
    bool spoken = false;
    QString speaker;
    QString text;
};

class Interaction final {
public:
    explicit Interaction(std::shared_ptr<database::DBConnection> connection);
    [[nodiscard]] TalkResult talk(const Catalog& catalog, const TalkContext& context,
                                  const TalkRequest& request) const;

private:
    std::shared_ptr<database::DBConnection> connection_;
};

} // namespace arcane::application::open_world::npc
