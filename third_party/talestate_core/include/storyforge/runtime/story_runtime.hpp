#pragma once

#include "storyforge/model/story_types.hpp"

#include <string>
#include <variant>
#include <vector>

namespace storyforge {

enum class RuntimeErrorCode {
    invalid_position,
    story_finished,
    unknown_choice,
    condition_unmet,
    invalid_effect,
    insufficient_resources,
    arithmetic_overflow
};

struct RuntimeError {
    RuntimeErrorCode code;
    std::string message;
};

using CommandResult = std::variant<StoryDelta, RuntimeError>;

[[nodiscard]] StoryState make_initial_state(const StoryPackage& package);
// Role attributes are starting values; declared world variables are the live authority.
[[nodiscard]] std::map<std::string, std::int64_t> current_player_attributes(
    const StoryPackage& package, const StoryState& state);
[[nodiscard]] const StoryScene* current_scene(const StoryPackage& package, const StoryState& state);
[[nodiscard]] const StoryNode* current_node(const StoryPackage& package, const StoryState& state);
[[nodiscard]] bool evaluate_condition(const StoryCondition& condition, const StoryState& state);
[[nodiscard]] bool choice_is_available(const StoryChoice& choice, const StoryState& state);

// All effects are checked on a private copy. On failure, state is unchanged.
// The caller receives the committed before/after snapshot and events on success.
[[nodiscard]] CommandResult execute_command(const StoryPackage& package, StoryState& state,
                                            const StoryCommand& command);

// Execute one player turn from author-approved, world-wide agent capabilities.
// The entire sequence is atomic. An empty sequence is a narration-only turn.
[[nodiscard]] CommandResult execute_agent_actions(const StoryPackage& package,
    StoryState& state, const std::vector<ChoiceId>& actions);

// A parameterized turn may mix author-defined actions with generic move/talk.
// The entire sequence is validated on a private state copy and committed once.
[[nodiscard]] CommandResult execute_agent_requests(const StoryPackage& package,
    StoryState& state, const std::vector<AgentActionRequest>& actions);

[[nodiscard]] const char* to_string(RuntimeErrorCode code) noexcept;

} // namespace storyforge
