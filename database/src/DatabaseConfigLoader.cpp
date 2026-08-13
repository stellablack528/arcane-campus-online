#include "DatabaseConfigLoader.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <string_view>

namespace arcane::database {
namespace {

std::string trim(std::string value)
{
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char character) {
        return std::isspace(character) != 0;
    }).base();
    return first < last ? std::string(first, last) : std::string{};
}

bool parsePort(const std::string& value, std::uint16_t& port)
{
    try {
        const auto parsed = std::stoul(value);
        if (parsed > std::numeric_limits<std::uint16_t>::max()) {
            return false;
        }
        port = static_cast<std::uint16_t>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace

std::optional<DBConfig> DatabaseConfigLoader::loadFromFile(const std::filesystem::path& path,
                                                            std::string* error)
{
    std::ifstream input(path);
    if (!input) {
        if (error) {
            *error = "Unable to open database configuration: " + path.string();
        }
        return std::nullopt;
    }

    DBConfig config;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        line = trim(std::move(line));
        if (line.empty() || line.starts_with('#') || line.starts_with(';')) {
            continue;
        }
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            if (error) {
                *error = "Invalid database configuration at line " + std::to_string(lineNumber);
            }
            return std::nullopt;
        }
        const auto key = trim(line.substr(0, separator));
        const auto value = trim(line.substr(separator + 1));
        if (key == "driver") {
            if (value == "mysql") {
                config.driver = DatabaseDriverKind::MySql;
            } else if (value == "noop") {
                config.driver = DatabaseDriverKind::Noop;
            } else {
                if (error) {
                    *error = "Unsupported database driver: " + value;
                }
                return std::nullopt;
            }
        } else if (key == "host") {
            config.host = value;
        } else if (key == "port") {
            if (!parsePort(value, config.port)) {
                if (error) {
                    *error = "Invalid database port at line " + std::to_string(lineNumber);
                }
                return std::nullopt;
            }
        } else if (key == "database") {
            config.database = value;
        } else if (key == "username") {
            config.username = value;
        } else if (key == "password") {
            config.password = value;
        } else if (key == "character_set") {
            config.characterSet = value;
        } else if (key == "client_library_path") {
            config.clientLibraryPath = value;
        }
    }

    if (const char* password = std::getenv("ARCANE_DB_PASSWORD")) {
        config.password = password;
    }
    return config;
}

} // namespace arcane::database
