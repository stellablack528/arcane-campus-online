#include "NoopDatabaseDriver.hpp"

namespace arcane::database {

bool NoopDatabaseDriver::connect(const DBConfig& config)
{
    if (config.driver == DatabaseDriverKind::MySql) {
        lastError_ = "MySQL support is not included in this build.";
        connected_ = false;
        return false;
    }
    connected_ = true;
    return connected_;
}

void NoopDatabaseDriver::disconnect() noexcept
{
    transactionActive_ = false;
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

bool NoopDatabaseDriver::beginTransaction()
{
    if (!connected_ || transactionActive_) return false;
    transactionActive_ = true;
    return true;
}

bool NoopDatabaseDriver::commitTransaction()
{
    if (!connected_ || !transactionActive_) return false;
    transactionActive_ = false;
    return true;
}

bool NoopDatabaseDriver::rollbackTransaction()
{
    if (!connected_ || !transactionActive_) return false;
    transactionActive_ = false;
    return true;
}

std::string_view NoopDatabaseDriver::driverName() const noexcept
{
    return "noop";
}

const std::string& NoopDatabaseDriver::lastError() const noexcept
{
    return lastError_;
}

} // namespace arcane::database
