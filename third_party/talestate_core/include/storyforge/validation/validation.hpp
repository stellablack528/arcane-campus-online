#pragma once

#include "storyforge/model/story_types.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace storyforge {

enum class IssueSeverity { warning, error };

struct ValidationIssue {
    IssueSeverity severity{IssueSeverity::error};
    std::string code;
    std::string path;
    std::string message;
};

struct ValidationReport {
    std::vector<ValidationIssue> issues;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::size_t error_count() const noexcept;
    [[nodiscard]] std::size_t warning_count() const noexcept;
    void add_error(std::string code, std::string path, std::string message);
    void add_warning(std::string code, std::string path, std::string message);
    void append(const ValidationReport& other);
};

struct LoadResult {
    std::optional<StoryPackage> package;
    ValidationReport report;
};

[[nodiscard]] ValidationReport validate_story_package(const StoryPackage& package);
[[nodiscard]] std::string to_string(IssueSeverity severity);

} // namespace storyforge
