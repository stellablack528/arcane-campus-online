#include "DBConnectionPool.hpp"

#include <algorithm>
#include <utility>

namespace arcane::database {

DBConnectionPool::PooledConnection::PooledConnection(DBConnectionPool* pool,
                                                     std::unique_ptr<DBConnection> connection) noexcept
    : pool_(pool)
    , connection_(std::move(connection))
{
}

DBConnectionPool::PooledConnection::~PooledConnection() noexcept
{
    release();
}

DBConnectionPool::PooledConnection::PooledConnection(PooledConnection&& other) noexcept
    : pool_(std::exchange(other.pool_, nullptr))
    , connection_(std::move(other.connection_))
{
}

DBConnectionPool::PooledConnection&
DBConnectionPool::PooledConnection::operator=(PooledConnection&& other) noexcept
{
    if (this != &other) {
        release();
        pool_ = std::exchange(other.pool_, nullptr);
        connection_ = std::move(other.connection_);
    }
    return *this;
}

DBConnection* DBConnectionPool::PooledConnection::operator->() const noexcept
{
    return connection_.get();
}

DBConnection& DBConnectionPool::PooledConnection::operator*() const noexcept
{
    return *connection_;
}

DBConnectionPool::PooledConnection::operator bool() const noexcept
{
    return connection_ != nullptr;
}

void DBConnectionPool::PooledConnection::release() noexcept
{
    if (pool_ && connection_) {
        pool_->release(std::move(connection_));
    }
    pool_ = nullptr;
}

DBConnectionPool::DBConnectionPool(DBConfig config, std::size_t initialSize, std::size_t maxSize)
    : config_(std::move(config))
    , maxSize_(std::max<std::size_t>(1, maxSize))
{
    const std::size_t size = std::min(initialSize, maxSize_);
    for (std::size_t index = 0; index < size; ++index) {
        available_.push(createConnection());
        ++totalConnections_;
    }
}

DBConnectionPool::~DBConnectionPool() noexcept
{
    shutdown();
}

DBConnectionPool::PooledConnection DBConnectionPool::acquire()
{
    std::unique_lock lock(mutex_);
    condition_.wait(lock, [this] {
        return shuttingDown_ || !available_.empty() || totalConnections_ < maxSize_;
    });

    if (shuttingDown_) {
        return {};
    }

    if (!available_.empty()) {
        auto connection = std::move(available_.front());
        available_.pop();
        return PooledConnection(this, std::move(connection));
    }

    ++totalConnections_;
    lock.unlock();
    return PooledConnection(this, createConnection());
}

void DBConnectionPool::shutdown() noexcept
{
    {
        std::lock_guard lock(mutex_);
        shuttingDown_ = true;
        while (!available_.empty()) {
            available_.pop();
        }
        totalConnections_ = 0;
    }
    condition_.notify_all();
}

std::size_t DBConnectionPool::availableCount() const noexcept
{
    std::lock_guard lock(mutex_);
    return available_.size();
}

std::size_t DBConnectionPool::maxSize() const noexcept
{
    return maxSize_;
}

void DBConnectionPool::release(std::unique_ptr<DBConnection> connection) noexcept
{
    if (!connection) {
        return;
    }

    {
        std::lock_guard lock(mutex_);
        if (shuttingDown_) {
            if (totalConnections_ > 0) {
                --totalConnections_;
            }
            return;
        }
        available_.push(std::move(connection));
    }
    condition_.notify_one();
}

std::unique_ptr<DBConnection> DBConnectionPool::createConnection()
{
    auto connection = std::make_unique<DBConnection>(config_);
    static_cast<void>(connection->connect());
    return connection;
}

} // namespace arcane::database
