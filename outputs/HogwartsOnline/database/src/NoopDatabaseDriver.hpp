#pragma once

#include "IDatabaseDriver.hpp"

namespace arcane::database {

class NoopDatabaseDriver final : public IDatabaseDriver {
public:
    [[nodiscard]] bool connect(const DBConfig& config) override;
    void disconnect() noexcept override;
    [[nodiscard]] bool isConnected() const noexcept override;

    [[nodiscard]] bool execute(const std::string& sql, const QueryParams& params) override;
    [[nodiscard]] std::optional<QueryResult> query(const std::string& sql,
                                                   const QueryParams& params) override;
    [[nodiscard]] std::string_view driverName() const noexcept override;

private:
    bool connected_ = false;
};

} // namespace arcane::database
