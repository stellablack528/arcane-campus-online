#pragma once

#include "DBConnection.hpp"

#include <optional>
#include <string>

namespace arcane::database {

class IDatabaseDriver {
public:
    virtual ~IDatabaseDriver() noexcept = default;

    [[nodiscard]] virtual bool connect(const DBConfig& config) = 0;
    virtual void disconnect() noexcept = 0;
    [[nodiscard]] virtual bool isConnected() const noexcept = 0;

    [[nodiscard]] virtual bool execute(const std::string& sql, const QueryParams& params) = 0;
    [[nodiscard]] virtual std::optional<QueryResult> query(const std::string& sql,
                                                           const QueryParams& params) = 0;
    [[nodiscard]] virtual std::string_view driverName() const noexcept = 0;
};

} // namespace arcane::database
