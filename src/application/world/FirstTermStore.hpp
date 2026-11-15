#pragma once

#include "application/world/FirstTermEngine.hpp"

#include <cstdint>
#include <memory>
#include <optional>

namespace arcane::database { class DBConnection; }

namespace arcane::application::world {

class FirstTermStore final {
public:
    explicit FirstTermStore(std::shared_ptr<database::DBConnection> connection);
    [[nodiscard]] std::optional<FirstTermState> load(std::uint64_t characterId) const;
    [[nodiscard]] bool save(std::uint64_t characterId, const FirstTermState& state) const;
    [[nodiscard]] bool ready() const noexcept { return ready_; }

private:
    std::shared_ptr<database::DBConnection> connection_;
    bool ready_ = false;
};

} // namespace arcane::application::world
