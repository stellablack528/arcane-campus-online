#include "NoopDatabaseDriver.hpp"

namespace arcane::database {

bool NoopDatabaseDriver::connect(const DBConfig& config)
{
    static_cast<void>(config);
    connected_ = true;
    return connected_;
}

void NoopDatabaseDriver::disconnect() noexcept
{
    connected_ = false;
}

bool NoopDatabaseDriver::isConnected() const noexcept
{
    return connected_;
}

bool NoopDatabaseDriver::execute(const std::string& sql, const QueryParams& params)
{
    static_cast<void>(sql);
    static_cast<void>(params);
    return connected_;
}

std::optional<QueryResult> NoopDatabaseDriver::query(const std::string& sql, const QueryParams& params)
{
    static_cast<void>(sql);
    static_cast<void>(params);
    if (!connected_) {
        return std::nullopt;
    }
    return QueryResult{};
}

std::string_view NoopDatabaseDriver::driverName() const noexcept
{
    return "noop";
}

} // namespace arcane::database
