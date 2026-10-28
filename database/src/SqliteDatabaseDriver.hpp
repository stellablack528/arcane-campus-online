#pragma once

#include "IDatabaseDriver.hpp"

#include <QSqlDatabase>
#include <QString>

namespace arcane::database {

class SqliteDatabaseDriver final : public IDatabaseDriver {
public:
    ~SqliteDatabaseDriver() noexcept override;

    [[nodiscard]] bool connect(const DBConfig& config) override;
    void disconnect() noexcept override;
    [[nodiscard]] bool isConnected() const noexcept override;
    [[nodiscard]] bool execute(const std::string& sql, const QueryParams& params) override;
    [[nodiscard]] std::optional<QueryResult> query(const std::string& sql,
                                                   const QueryParams& params) override;
    [[nodiscard]] bool beginTransaction() override;
    [[nodiscard]] bool commitTransaction() override;
    [[nodiscard]] bool rollbackTransaction() override;
    [[nodiscard]] std::string_view driverName() const noexcept override;
    [[nodiscard]] const std::string& lastError() const noexcept override;

private:
    [[nodiscard]] QString sqliteSql(const std::string& sql) const;
    [[nodiscard]] bool runControl(const char* sql);

    QString connectionName_;
    QSqlDatabase database_;
    std::string lastError_;
    bool transactionActive_ = false;
};

} // namespace arcane::database
