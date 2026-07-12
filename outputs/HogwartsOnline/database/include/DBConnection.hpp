#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace arcane::database {

struct DBConfig {
    std::string host = "127.0.0.1";
    std::uint16_t port = 3306;
    std::string database = "arcane_campus_online";
    std::string username = "root";
    std::string password;
    std::chrono::milliseconds connectTimeout{3000};
};

class IDatabaseDriver;

using QueryParams = std::vector<std::string>;
using QueryRow = std::unordered_map<std::string, std::string>;

struct QueryResult {
    std::vector<QueryRow> rows;
    std::uint64_t affectedRows = 0;
    std::uint64_t lastInsertId = 0;
};

class DBConnection final {
public:
    explicit DBConnection(DBConfig config);
    ~DBConnection() noexcept;

    DBConnection(const DBConnection&) = delete;
    DBConnection& operator=(const DBConnection&) = delete;
    DBConnection(DBConnection&& other) noexcept;
    DBConnection& operator=(DBConnection&& other) noexcept;

    [[nodiscard]] bool connect();
    void disconnect() noexcept;
    [[nodiscard]] bool isConnected() const noexcept;
    [[nodiscard]] const DBConfig& config() const noexcept;
    [[nodiscard]] std::string_view driverName() const noexcept;

    [[nodiscard]] bool execute(const std::string& sql, const QueryParams& params = {});
    [[nodiscard]] std::optional<QueryResult> query(const std::string& sql, const QueryParams& params = {});

private:
    DBConfig config_;
    std::unique_ptr<IDatabaseDriver> driver_;
};

} // namespace arcane::database
