#include "LocalSaveBootstrap.hpp"

#include "DBTransaction.hpp"

#include <QFile>
#include <QString>
#include <QStringList>

namespace arcane::database {

std::shared_ptr<DBConnection> LocalSaveBootstrap::open(
    const std::filesystem::path& path, std::string* error)
{
    std::error_code filesystemError;
    std::filesystem::create_directories(path.parent_path(), filesystemError);
    if (filesystemError) {
        if (error) *error = "Could not create the local save folder: " + filesystemError.message();
        return nullptr;
    }

    DBConfig config;
    config.driver = DatabaseDriverKind::Sqlite;
    const auto utf8 = path.u8string();
    config.database.assign(utf8.begin(), utf8.end());
    auto connection = std::make_shared<DBConnection>(std::move(config));
    if (!connection->connect()) {
        if (error) *error = connection->lastError();
        return nullptr;
    }

    const auto version = connection->query("PRAGMA user_version");
    if (!version || version->rows.empty()) {
        if (error) *error = connection->lastError();
        return nullptr;
    }
    const auto versionText = version->rows.front().at("user_version");
    if (versionText != "0" && versionText != "1") {
        if (error) *error = "This local save was created by a newer game version.";
        return nullptr;
    }

    QFile schema(QStringLiteral(":/database/local_save_v1.sql"));
    if (!schema.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = "The local save schema is missing from the game build.";
        return nullptr;
    }
    DBTransaction transaction(connection);
    if (!transaction.isActive()) {
        if (error) *error = connection->lastError();
        return nullptr;
    }

    QString statement;
    bool trigger = false;
    const auto lines = QString::fromUtf8(schema.readAll()).split(QLatin1Char('\n'));
    for (const auto& line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;
        if (statement.isEmpty() && trimmed.startsWith(QLatin1String("CREATE TRIGGER"),
                                                      Qt::CaseInsensitive)) trigger = true;
        statement += line + QLatin1Char('\n');
        if (!trimmed.endsWith(QLatin1Char(';'))
            || (trigger && trimmed.compare(QLatin1String("END;"), Qt::CaseInsensitive) != 0))
            continue;
        statement = statement.trimmed();
        statement.chop(1);
        if (!connection->execute(statement.toStdString())) {
            if (error) *error = connection->lastError();
            return nullptr;
        }
        statement.clear();
        trigger = false;
    }
    if (!statement.trimmed().isEmpty() || !transaction.commit()) {
        if (error) *error = connection->lastError().empty()
            ? "The local save schema is incomplete." : connection->lastError();
        return nullptr;
    }
    return connection;
}

} // namespace arcane::database
