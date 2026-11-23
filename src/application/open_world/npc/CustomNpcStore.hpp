#pragma once

#include "application/open_world/npc/NpcCatalog.hpp"
#include <cstdint>

namespace arcane::application::open_world::npc {

// A single replaceable pack per local character and era. Never rewrites the built-in content.
class CustomNpcStore final {
public:
    explicit CustomNpcStore(QString root);
    [[nodiscard]] bool restore(std::uint64_t character, const QString& era, const Catalog& builtIns,
                               Catalog& result, QString& error) const;
    [[nodiscard]] bool importPack(std::uint64_t character, const QString& era, const QByteArray& json,
                                  const Catalog& builtIns, Catalog& result, QString& error) const;
    static constexpr qsizetype maxBytes = 256 * 1024;

private:
    [[nodiscard]] QString filePath(std::uint64_t character, const QString& era) const;
    QString root_;
};

} // namespace arcane::application::open_world::npc
