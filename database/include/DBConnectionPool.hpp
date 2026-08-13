#pragma once

#include "DBConnection.hpp"

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>

namespace arcane::database {

class DBConnectionPool final {
public:
    class PooledConnection final {
    public:
        PooledConnection() noexcept = default;
        PooledConnection(DBConnectionPool* pool, std::unique_ptr<DBConnection> connection) noexcept;
        ~PooledConnection() noexcept;

        PooledConnection(const PooledConnection&) = delete;
        PooledConnection& operator=(const PooledConnection&) = delete;
        PooledConnection(PooledConnection&& other) noexcept;
        PooledConnection& operator=(PooledConnection&& other) noexcept;

        [[nodiscard]] DBConnection* operator->() const noexcept;
        [[nodiscard]] DBConnection& operator*() const noexcept;
        [[nodiscard]] explicit operator bool() const noexcept;

    private:
        void release() noexcept;

        DBConnectionPool* pool_ = nullptr;
        std::unique_ptr<DBConnection> connection_;
    };

    DBConnectionPool(DBConfig config, std::size_t initialSize, std::size_t maxSize);
    ~DBConnectionPool() noexcept;

    DBConnectionPool(const DBConnectionPool&) = delete;
    DBConnectionPool& operator=(const DBConnectionPool&) = delete;

    [[nodiscard]] PooledConnection acquire();
    void shutdown() noexcept;

    [[nodiscard]] std::size_t availableCount() const noexcept;
    [[nodiscard]] std::size_t maxSize() const noexcept;

private:
    friend class PooledConnection;

    void release(std::unique_ptr<DBConnection> connection) noexcept;
    [[nodiscard]] std::unique_ptr<DBConnection> createConnection();

    DBConfig config_;
    std::size_t maxSize_ = 0;
    std::size_t totalConnections_ = 0;
    bool shuttingDown_ = false;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<std::unique_ptr<DBConnection>> available_;
};

} // namespace arcane::database
