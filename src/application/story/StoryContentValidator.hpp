#pragma once

#include "application/story/StoryTypes.hpp"

#include <QStringList>

namespace arcane::application::story {

struct StoryValidationResult {
    QStringList errors;
    QStringList warnings;

    [[nodiscard]] bool isValid() const { return errors.isEmpty(); }
};

class StoryContentValidator final {
public:
    [[nodiscard]] StoryValidationResult validate(const ChapterDefinition& chapter) const;
};

} // namespace arcane::application::story
