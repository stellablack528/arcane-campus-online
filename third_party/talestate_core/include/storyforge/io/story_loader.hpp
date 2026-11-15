#pragma once

#include "storyforge/validation/validation.hpp"

#include <filesystem>
#include <string>

namespace storyforge {

// A directory resolves to <directory>/story.json; a JSON file is used directly.
[[nodiscard]] LoadResult load_story_package(const std::filesystem::path& path);
[[nodiscard]] LoadResult load_story_package_text(const std::string& json_text);

} // namespace storyforge
