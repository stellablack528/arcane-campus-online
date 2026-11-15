#include "application/open_world/talestate/OpenWorldRouteGuard.hpp"

#include <storyforge/runtime/story_runtime.hpp>

#include <array>
#include <string_view>
#include <utility>
#include <variant>

namespace arcane::application::open_world::talestate {
namespace {

constexpr std::array<const char*, 9> kScenes{
    "charms_classroom", "great_hall", "transfiguration_classroom",
    "library", "courtyard", "hogsmeade", "greenhouse_three",
    "defense_classroom", "forbidden_forest"};
constexpr std::array<const char*, 8> kDestinations{
    "great_hall", "transfiguration_classroom", "library", "courtyard",
    "hogsmeade", "charms_classroom", "greenhouse_three", "defense_classroom"};

storyforge::StoryPackage makePackage()
{
    storyforge::StoryPackage package;
    package.format = "story-package/v1";
    package.id = storyforge::StoryId{"arcane-campus-open-world"};
    package.title = "Hogwarts campus exploration";
    package.version = "1";
    const storyforge::ChapterId chapterId{"open_world"};
    const storyforge::NodeId arrivalId{"arrival"};
    package.entry = {chapterId, storyforge::SceneId{"great_hall"}, arrivalId};

    storyforge::StoryChapter chapter;
    chapter.id = chapterId;
    chapter.title = package.title;
    chapter.entry_scene = package.entry.scene;
    for (const char* id : kScenes) {
        storyforge::StoryScene scene;
        scene.id = storyforge::SceneId{id};
        scene.title = id;
        scene.entry_node = arrivalId;
        scene.nodes.push_back({arrivalId, "", {}, false});
        chapter.scenes.push_back(std::move(scene));
    }
    package.chapters.push_back(std::move(chapter));
    for (const char* from : kScenes) {
        for (const char* to : kDestinations) {
            if (std::string_view(from) == "forbidden_forest" && std::string_view(to) != "courtyard") continue;
            if (std::string_view(from) != to)
                package.agent_routes.push_back({storyforge::SceneId{from}, storyforge::SceneId{to}, {}});
        }
    }
    package.agent_routes.push_back({storyforge::SceneId{"courtyard"},
                                    storyforge::SceneId{"forbidden_forest"}, {}});
    return package;
}

} // namespace

RouteDecision OpenWorldRouteGuard::check(const std::string& currentSceneId,
                                         const std::string& destinationSceneId)
{
    static const auto package = makePackage();
    auto state = storyforge::make_initial_state(package);
    state.position.scene = storyforge::SceneId{currentSceneId};
    state.position.node = storyforge::NodeId{"arrival"};
    if (!storyforge::current_node(package, state))
        return {false, "Current scene is not part of the open world."};
    if (currentSceneId == destinationSceneId) return {true, {}};
    const auto result = storyforge::execute_agent_requests(package, state,
        {{storyforge::AgentActionKind::move, destinationSceneId, {}}});
    if (const auto* error = std::get_if<storyforge::RuntimeError>(&result))
        return {false, error->message};
    return {state.position.scene.value() == destinationSceneId, {}};
}

} // namespace arcane::application::open_world::talestate
