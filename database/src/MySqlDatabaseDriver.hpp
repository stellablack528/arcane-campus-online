#pragma once

#include "IDatabaseDriver.hpp"

#include <mysql.h>

#include <windows.h>

#include <memory>

namespace arcane::database {

class MySqlDatabaseDriver final : public IDatabaseDriver {
public:
    MySqlDatabaseDriver();
    ~MySqlDatabaseDriver() noexcept override;

    MySqlDatabaseDriver(const MySqlDatabaseDriver&) = delete;
    MySqlDatabaseDriver& operator=(const MySqlDatabaseDriver&) = delete;

    [[nodiscard]] bool connect(const DBConfig& config) override;
    void disconnect() noexcept override;
    [[nodiscard]] bool isConnected() const noexcept override;
    [[nodiscard]] bool execute(const std::string& sql, const QueryParams& params) override;
    [[nodiscard]] std::optional<QueryResult> query(const std::string& sql,
                                                    const QueryParams& params) override;
    [[nodiscard]] std::string_view driverName() const noexcept override;
    [[nodiscard]] const std::string& lastError() const noexcept override;

private:
    struct Api;

    [[nodiscard]] bool loadApi(const DBConfig& config);
    [[nodiscard]] std::optional<std::string> bind(const std::string& sql, const QueryParams& params);
    void setErrorFromServer();

    HMODULE module_ = nullptr;
    MYSQL* connection_ = nullptr;
    std::unique_ptr<Api> api_;
    std::string lastError_;
};

} // namespace arcane::database
