#include "SqliteDatabaseDriver.hpp"

#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

namespace arcane::database {

SqliteDatabaseDriver::~SqliteDatabaseDriver() noexcept
{
    disconnect();
}

bool SqliteDatabaseDriver::connect(const DBConfig& config)
{
    disconnect();
    lastError_.clear();
    connectionName_ = QStringLiteral("arcane-local-%1")
                          .arg(reinterpret_cast<quintptr>(this));
    database_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName_);
    database_.setDatabaseName(QString::fromStdString(config.database));
    if (!database_.open()) {
        lastError_ = database_.lastError().text().toStdString();
        disconnect();
        return false;
    }
    if (!execute("PRAGMA foreign_keys = ON", {})
        || !execute("PRAGMA busy_timeout = 5000", {})) {
        disconnect();
        return false;
    }
    return true;
}

void SqliteDatabaseDriver::disconnect() noexcept
{
    if (connectionName_.isEmpty()) return;
    database_.close();
    database_ = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName_);
    connectionName_.clear();
    transactionActive_ = false;
}

bool SqliteDatabaseDriver::isConnected() const noexcept
{
    return database_.isValid() && database_.isOpen();
}

QString SqliteDatabaseDriver::sqliteSql(const std::string& sql) const
{
    QString text = QString::fromStdString(sql);
    text.remove(QRegularExpression(QStringLiteral(R"(\s+FOR UPDATE\b)"),
                                   QRegularExpression::CaseInsensitiveOption));
    const QString marker = QStringLiteral(" ON DUPLICATE KEY UPDATE ");
    const qsizetype markerAt = text.indexOf(marker, 0, Qt::CaseInsensitive);
    if (markerAt >= 0) {
        QString updates = text.mid(markerAt + marker.size());
        updates.replace(QRegularExpression(QStringLiteral(R"(VALUES\(([A-Za-z_][A-Za-z0-9_]*)\))"),
                                           QRegularExpression::CaseInsensitiveOption),
                        QStringLiteral("excluded.\\1"));
        text = text.left(markerAt) + QStringLiteral(" ON CONFLICT DO UPDATE SET ") + updates;
    }
    return text;
}

bool SqliteDatabaseDriver::execute(const std::string& sql, const QueryParams& params)
{
    if (!isConnected()) return false;
    QSqlQuery statement(database_);
    if (!statement.prepare(sqliteSql(sql))) {
        lastError_ = statement.lastError().text().toStdString();
        return false;
    }
    for (const auto& param : params) statement.addBindValue(QString::fromStdString(param));
    if (!statement.exec()) {
        lastError_ = statement.lastError().text().toStdString();
        return false;
    }
    lastError_.clear();
    return true;
}

std::optional<QueryResult> SqliteDatabaseDriver::query(const std::string& sql,
                                                        const QueryParams& params)
{
    if (!isConnected()) return std::nullopt;
    QSqlQuery statement(database_);
    if (!statement.prepare(sqliteSql(sql))) {
        lastError_ = statement.lastError().text().toStdString();
        return std::nullopt;
    }
    for (const auto& param : params) statement.addBindValue(QString::fromStdString(param));
    if (!statement.exec()) {
        lastError_ = statement.lastError().text().toStdString();
        return std::nullopt;
    }
    QueryResult result;
    result.affectedRows = statement.numRowsAffected() > 0
        ? static_cast<std::uint64_t>(statement.numRowsAffected()) : 0;
    result.lastInsertId = statement.lastInsertId().toULongLong();
    const auto columns = statement.record();
    while (statement.next()) {
        QueryRow row;
        for (int index = 0; index < columns.count(); ++index) {
            const auto value = statement.value(index);
            row.emplace(columns.fieldName(index).toStdString(),
                        value.isNull() ? std::string{} : value.toString().toStdString());
        }
        result.rows.push_back(std::move(row));
    }
    lastError_.clear();
    return result;
}

bool SqliteDatabaseDriver::runControl(const char* sql)
{
    if (!isConnected()) return false;
    QSqlQuery statement(database_);
    if (!statement.exec(QString::fromLatin1(sql))) {
        lastError_ = statement.lastError().text().toStdString();
        return false;
    }
    lastError_.clear();
    return true;
}

bool SqliteDatabaseDriver::beginTransaction()
{
    if (transactionActive_ || !runControl("BEGIN IMMEDIATE")) return false;
    transactionActive_ = true;
    return true;
}

bool SqliteDatabaseDriver::commitTransaction()
{
    if (!transactionActive_ || !runControl("COMMIT")) return false;
    transactionActive_ = false;
    return true;
}

bool SqliteDatabaseDriver::rollbackTransaction()
{
    if (!transactionActive_ || !runControl("ROLLBACK")) return false;
    transactionActive_ = false;
    return true;
}

std::string_view SqliteDatabaseDriver::driverName() const noexcept
{
    return "sqlite";
}

const std::string& SqliteDatabaseDriver::lastError() const noexcept
{
    return lastError_;
}

} // namespace arcane::database
