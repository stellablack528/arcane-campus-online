#include "MySqlDatabaseDriver.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>

namespace arcane::database {

struct MySqlDatabaseDriver::Api {
    decltype(&mysql_init) init = nullptr;
    decltype(&mysql_options) options = nullptr;
    decltype(&mysql_real_connect) realConnect = nullptr;
    decltype(&mysql_close) close = nullptr;
    decltype(&mysql_ping) ping = nullptr;
    decltype(&mysql_query) query = nullptr;
    decltype(&mysql_store_result) storeResult = nullptr;
    decltype(&mysql_free_result) freeResult = nullptr;
    decltype(&mysql_num_fields) numFields = nullptr;
    decltype(&mysql_fetch_fields) fetchFields = nullptr;
    decltype(&mysql_fetch_row) fetchRow = nullptr;
    decltype(&mysql_fetch_lengths) fetchLengths = nullptr;
    decltype(&mysql_affected_rows) affectedRows = nullptr;
    decltype(&mysql_insert_id) insertId = nullptr;
    decltype(&mysql_error) error = nullptr;
    decltype(&mysql_real_escape_string) escapeString = nullptr;
    decltype(&mysql_set_character_set) setCharacterSet = nullptr;
};

namespace {

template <typename Function>
bool loadFunction(HMODULE module, const char* name, Function& function)
{
    function = reinterpret_cast<Function>(GetProcAddress(module, name));
    return function != nullptr;
}

} // namespace

MySqlDatabaseDriver::MySqlDatabaseDriver() = default;

MySqlDatabaseDriver::~MySqlDatabaseDriver() noexcept
{
    disconnect();
}

bool MySqlDatabaseDriver::connect(const DBConfig& config)
{
    disconnect();
    if (!loadApi(config)) {
        return false;
    }

    connection_ = api_->init(nullptr);
    if (!connection_) {
        lastError_ = "mysql_init failed.";
        return false;
    }

    const auto timeoutSeconds = static_cast<unsigned int>(
        std::min<std::int64_t>(std::max<std::int64_t>(1, config.connectTimeout.count() / 1000),
                               std::numeric_limits<unsigned int>::max()));
    api_->options(connection_, MYSQL_OPT_CONNECT_TIMEOUT, &timeoutSeconds);

    if (!api_->realConnect(connection_,
                           config.host.c_str(),
                           config.username.c_str(),
                           config.password.c_str(),
                           config.database.empty() ? nullptr : config.database.c_str(),
                           config.port,
                           nullptr,
                           0)) {
        setErrorFromServer();
        api_->close(connection_);
        connection_ = nullptr;
        return false;
    }

    if (!config.characterSet.empty() && api_->setCharacterSet(connection_, config.characterSet.c_str()) != 0) {
        setErrorFromServer();
        disconnect();
        return false;
    }

    lastError_.clear();
    return true;
}

void MySqlDatabaseDriver::disconnect() noexcept
{
    if (connection_ && api_ && api_->close) {
        api_->close(connection_);
    }
    connection_ = nullptr;
    api_.reset();
    if (module_) {
        FreeLibrary(module_);
        module_ = nullptr;
    }
}

bool MySqlDatabaseDriver::isConnected() const noexcept
{
    return connection_ != nullptr && api_ && api_->ping(connection_) == 0;
}

bool MySqlDatabaseDriver::execute(const std::string& sql, const QueryParams& params)
{
    const auto boundSql = bind(sql, params);
    if (!boundSql || api_->query(connection_, boundSql->c_str()) != 0) {
        if (boundSql) {
            setErrorFromServer();
        }
        return false;
    }
    lastError_.clear();
    return true;
}

std::optional<QueryResult> MySqlDatabaseDriver::query(const std::string& sql, const QueryParams& params)
{
    const auto boundSql = bind(sql, params);
    if (!boundSql || api_->query(connection_, boundSql->c_str()) != 0) {
        if (boundSql) {
            setErrorFromServer();
        }
        return std::nullopt;
    }

    MYSQL_RES* result = api_->storeResult(connection_);
    if (!result) {
        lastError_.clear();
        return QueryResult{{}, api_->affectedRows(connection_), api_->insertId(connection_)};
    }

    QueryResult queryResult;
    const unsigned int fieldCount = api_->numFields(result);
    MYSQL_FIELD* fields = api_->fetchFields(result);
    while (MYSQL_ROW row = api_->fetchRow(result)) {
        const auto* lengths = api_->fetchLengths(result);
        QueryRow queryRow;
        for (unsigned int index = 0; index < fieldCount; ++index) {
            queryRow.emplace(fields[index].name,
                             row[index] ? std::string(row[index], lengths[index]) : std::string{});
        }
        queryResult.rows.push_back(std::move(queryRow));
    }
    api_->freeResult(result);
    queryResult.affectedRows = api_->affectedRows(connection_);
    queryResult.lastInsertId = api_->insertId(connection_);
    lastError_.clear();
    return queryResult;
}

std::string_view MySqlDatabaseDriver::driverName() const noexcept
{
    return "mysql";
}

const std::string& MySqlDatabaseDriver::lastError() const noexcept
{
    return lastError_;
}

bool MySqlDatabaseDriver::loadApi(const DBConfig& config)
{
    const std::string libraryPath = config.clientLibraryPath.empty()
        ? "libmysql.dll"
        : config.clientLibraryPath;
    module_ = LoadLibraryA(libraryPath.c_str());
    if (!module_) {
        lastError_ = "Unable to load MySQL client library: " + libraryPath;
        return false;
    }

    api_ = std::make_unique<Api>();
    const bool loaded = loadFunction(module_, "mysql_init", api_->init)
        && loadFunction(module_, "mysql_options", api_->options)
        && loadFunction(module_, "mysql_real_connect", api_->realConnect)
        && loadFunction(module_, "mysql_close", api_->close)
        && loadFunction(module_, "mysql_ping", api_->ping)
        && loadFunction(module_, "mysql_query", api_->query)
        && loadFunction(module_, "mysql_store_result", api_->storeResult)
        && loadFunction(module_, "mysql_free_result", api_->freeResult)
        && loadFunction(module_, "mysql_num_fields", api_->numFields)
        && loadFunction(module_, "mysql_fetch_fields", api_->fetchFields)
        && loadFunction(module_, "mysql_fetch_row", api_->fetchRow)
        && loadFunction(module_, "mysql_fetch_lengths", api_->fetchLengths)
        && loadFunction(module_, "mysql_affected_rows", api_->affectedRows)
        && loadFunction(module_, "mysql_insert_id", api_->insertId)
        && loadFunction(module_, "mysql_error", api_->error)
        && loadFunction(module_, "mysql_real_escape_string", api_->escapeString)
        && loadFunction(module_, "mysql_set_character_set", api_->setCharacterSet);
    if (!loaded) {
        lastError_ = "The MySQL client library is missing a required C API symbol.";
        disconnect();
        return false;
    }
    return true;
}

std::optional<std::string> MySqlDatabaseDriver::bind(const std::string& sql, const QueryParams& params)
{
    if (!connection_ || !api_) {
        lastError_ = "MySQL connection is not open.";
        return std::nullopt;
    }

    std::string boundSql;
    boundSql.reserve(sql.size() + params.size() * 8);
    std::size_t parameterIndex = 0;
    for (const char character : sql) {
        if (character != '?') {
            boundSql.push_back(character);
            continue;
        }
        if (parameterIndex >= params.size()) {
            lastError_ = "SQL placeholder count does not match parameter count.";
            return std::nullopt;
        }
        const auto& parameter = params[parameterIndex++];
        std::string escaped(parameter.size() * 2 + 1, '\0');
        const auto escapedLength = api_->escapeString(connection_, escaped.data(), parameter.data(), parameter.size());
        escaped.resize(escapedLength);
        boundSql.append("'").append(escaped).append("'");
    }
    if (parameterIndex != params.size()) {
        lastError_ = "SQL placeholder count does not match parameter count.";
        return std::nullopt;
    }
    return boundSql;
}

void MySqlDatabaseDriver::setErrorFromServer()
{
    lastError_ = connection_ && api_ && api_->error ? api_->error(connection_) : "Unknown MySQL error.";
}

} // namespace arcane::database
