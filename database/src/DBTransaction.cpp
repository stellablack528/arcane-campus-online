#include "DBTransaction.hpp"

#include "DBConnection.hpp"

#include <utility>

namespace arcane::database {

DBTransaction::DBTransaction(std::shared_ptr<DBConnection> connection)
    : connection_(std::move(connection))
    , active_(connection_ && connection_->beginTransaction())
{
}

DBTransaction::~DBTransaction() noexcept
{
    if (active_) {
        try {
            (void)connection_->rollbackTransaction();
        } catch (...) {
            // Destructors must not leak persistence errors during stack unwinding.
        }
    }
}

bool DBTransaction::commit()
{
    if (!active_ || !connection_) return false;
    if (!connection_->commitTransaction()) return false;
    active_ = false;
    return true;
}

bool DBTransaction::rollback()
{
    if (!active_ || !connection_) return false;
    const bool success = connection_->rollbackTransaction();
    if (success) active_ = false;
    return success;
}

} // namespace arcane::database
