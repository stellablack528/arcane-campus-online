#pragma once

#include "DBConnection.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace arcane::database {

class DatabaseConfigLoader final {
public:
    [[nodiscard]] static std::optional<DBConfig> loadFromFile(const std::filesystem::path& path,
                                                               std::string* error = nullptr);
};

} // namespace arcane::database
