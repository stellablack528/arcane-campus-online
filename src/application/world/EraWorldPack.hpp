#pragma once

#include <QString>
#include <memory>

namespace storyforge { struct StoryPackage; }

namespace arcane::application::world {

class EraWorldPack final {
public:
    [[nodiscard]] static std::shared_ptr<const storyforge::StoryPackage> load(const QString& eraId,
                                                                               QString* error = nullptr);
    [[nodiscard]] static QString title(const storyforge::StoryPackage& package);
    [[nodiscard]] static QString classmate(const storyforge::StoryPackage& package);
    [[nodiscard]] static QString professor(const storyforge::StoryPackage& package);
};

} // namespace arcane::application::world
