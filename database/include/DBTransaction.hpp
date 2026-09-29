#pragma once

#include <memory>

namespace arcane::database {

class DBConnection;

// RAII transaction guard. An unfinished transaction is always rolled back.
class DBTransaction final {
public:
    explicit DBTransaction(std::shared_ptr<DBConnection> connection);
    ~DBTransaction() noexcept;

    DBTransaction(const DBTransaction&) = delete;
    DBTransaction& operator=(const DBTransaction&) = delete;
    DBTransaction(DBTransaction&&) = delete;
    DBTransaction& operator=(DBTransaction&&) = delete;

    [[nodiscard]] bool isActive() const noexcept { return active_; }
    [[nodiscard]] bool commit();
    [[nodiscard]] bool rollback();

private:
    std::shared_ptr<DBConnection> connection_;
    bool active_ = false;
};

} // namespace arcane::database
