#include "DBConnection.hpp"

#include "NoopDatabaseDriver.hpp"

#include <utility>

namespace arcane::database {

DBConnection::DBConnection(DBConfig config)
    : config_(std::move(config))
    , driver_(std::make_unique<NoopDatabaseDriver>())
{
}

DBConnection::~DBConnection() noexcept
{
    disconnect();
}

DBConnection::DBConnection(DBConnection&& other) noexcept
    : config_(std::move(other.config_))
    , driver_(std::move(other.driver_))
{
}

DBConnection& DBConnection::operator=(DBConnection&& other) noexcept
{
    if (this != &other) {
        disconnect();
        config_ = std::move(other.config_);
        driver_ = std::move(other.driver_);
    }
    return *this;
}

bool DBConnection::connect()
{
    return driver_ && driver_->connect(config_);
}

void DBConnection::disconnect() noexcept
{
    if (driver_) {
        driver_->disconnect();
    }
}

bool DBConnection::isConnected() const noexcept
{
    return driver_ && driver_->isConnected();
}

const DBConfig& DBConnection::config() const noexcept
{
    return config_;
}

std::string_view DBConnection::driverName() const noexcept
{
    return driver_ ? driver_->driverName() : "none";
}

bool DBConnection::execute(const std::string& sql, const QueryParams& params)
{
    return driver_ && driver_->execute(sql, params);
}

std::optional<QueryResult> DBConnection::query(const std::string& sql, const QueryParams& params)
{
    if (!driver_) {
        return std::nullopt;
    }
    return driver_->query(sql, params);
}

} // namespace arcane::database
