#pragma once

#include "application/story/StoryTypes.hpp"

#include <QByteArray>
#include <QString>
#include <QStringList>

#include <optional>

namespace arcane::application::story {

struct StoryLoadResult {
    std::optional<ChapterDefinition> chapter;
    QStringList errors;

    [[nodiscard]] bool success() const { return chapter.has_value() && errors.isEmpty(); }
};

class StoryContentLoader final {
public:
    [[nodiscard]] StoryLoadResult loadFile(const QString& path) const;
    [[nodiscard]] StoryLoadResult loadJson(const QByteArray& json,
                                           const QString& sourceName = QStringLiteral("memory")) const;
};

} // namespace arcane::application::story
