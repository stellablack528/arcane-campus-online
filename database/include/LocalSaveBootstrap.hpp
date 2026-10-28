#pragma once

#include "DBConnection.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace arcane::database {

class LocalSaveBootstrap final {
public:
    [[nodiscard]] static std::shared_ptr<DBConnection> open(
        const std::filesystem::path& path, std::string* error = nullptr);
};

} // namespace arcane::database
