#include "application/open_world/npc/CustomNpcStore.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <utility>

namespace arcane::application::open_world::npc {
using namespace Qt::StringLiterals;

CustomNpcStore::CustomNpcStore(QString root) : root_(std::move(root)) {}

QString CustomNpcStore::filePath(std::uint64_t character, const QString& era) const
{
    static const QRegularExpression eraId(u"^[a-z][a-z0-9_]{0,30}$"_s);
    if (!character || root_.isEmpty() || !eraId.match(era).hasMatch()) return {};
    return QDir(root_).filePath(u"npc-packs/%1/%2.json"_s.arg(QString::number(character), era));
}

bool CustomNpcStore::restore(std::uint64_t character, const QString& era, const Catalog& builtIns,
                             Catalog& result, QString& error) const
{
    const auto path = filePath(character, era);
    error.clear();
    if (path.isEmpty()) { error = u"Invalid local NPC pack location."_s; return false; }
    Catalog candidate = builtIns;
    QFile file(path);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) { error = file.errorString(); return false; }
        const auto json = file.read(maxBytes + 1);
        if (file.error() != QFileDevice::NoError) { error = file.errorString(); return false; }
        if (!candidate.loadCustomPack(json, era, error)) return false;
    }
    result = std::move(candidate);
    return true;
}

bool CustomNpcStore::importPack(std::uint64_t character, const QString& era, const QByteArray& json,
                                const Catalog& builtIns, Catalog& result, QString& error) const
{
    const auto path = filePath(character, era);
    if (path.isEmpty()) { error = u"Invalid local NPC pack location."_s; return false; }
    Catalog candidate = builtIns;
    if (!candidate.loadCustomPack(json, era, error)) return false;
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) { error = u"Could not create the NPC pack folder."_s; return false; }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(json) != json.size() || !file.commit()) {
        error = file.errorString();
        return false;
    }
    result = std::move(candidate);
    return true;
}

} // namespace arcane::application::open_world::npc
