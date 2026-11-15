#include "storyforge/validation/validation.hpp"

#include <algorithm>
#include <deque>
#include <map>
#include <set>
#include <sstream>
#include <tuple>
#include <type_traits>

namespace storyforge {
namespace {

template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

struct Location {
    const StoryChapter* chapter{};
    const StoryScene* scene{};
    const StoryNode* node{};
    std::string path;
};

std::string key(const ChapterId& chapter, const SceneId& scene, const NodeId& node) {
    return chapter.value() + "\x1f" + scene.value() + "\x1f" + node.value();
}

void require_unique(std::set<std::string>& values, const std::string& value,
                    const std::string& path, const std::string& kind,
                    ValidationReport& report) {
    if (!values.insert(value).second)
        report.add_error("semantic.duplicate_id", path, "duplicate " + kind + " id '" + value + "'");
}

const StoryChapter* find_chapter(const StoryPackage& package, const ChapterId& id) {
    const auto it = std::find_if(package.chapters.begin(), package.chapters.end(),
                                 [&](const auto& value) { return value.id == id; });
    return it == package.chapters.end() ? nullptr : &*it;
}

const StoryScene* find_scene(const StoryChapter& chapter, const SceneId& id) {
    const auto it = std::find_if(chapter.scenes.begin(), chapter.scenes.end(),
                                 [&](const auto& value) { return value.id == id; });
    return it == chapter.scenes.end() ? nullptr : &*it;
}

const StoryNode* find_node(const StoryScene& scene, const NodeId& id) {
    const auto it = std::find_if(scene.nodes.begin(), scene.nodes.end(),
                                 [&](const auto& value) { return value.id == id; });
    return it == scene.nodes.end() ? nullptr : &*it;
}

void validate_condition(const StoryCondition& condition, const std::string& path,
                        const std::map<std::string, StoryValue>& variables,
                        const std::set<std::string>& scenes,
                        ValidationReport& report) {
    std::visit(Overloaded{
        [&](const VariableComparison& value) {
            const auto found = variables.find(value.variable.value());
            if (found == variables.end())
                report.add_error("semantic.missing_reference", path + "/variable",
                                 "unknown variable '" + value.variable.value() + "'");
            else if (found->second.index() != value.value.index())
                report.add_error("semantic.invalid_condition", path + "/value",
                                 "comparison value must match the variable type");
            else if (value.op != CompareOperator::equal && value.op != CompareOperator::not_equal &&
                     !std::holds_alternative<std::int64_t>(found->second) &&
                     !std::holds_alternative<double>(found->second))
                report.add_error("semantic.invalid_condition", path + "/op",
                                 "ordered comparison requires a numeric variable");
        },
        [&](const CurrentSceneCondition& value) {
            if (!scenes.contains(value.scene.value()))
                report.add_error("semantic.missing_reference", path + "/scene",
                                 "unknown scene '" + value.scene.value() + "'");
        },
        [&](const AllCondition& value) {
            for (std::size_t i = 0; i < value.conditions.size(); ++i) {
                if (value.conditions[i]) validate_condition(*value.conditions[i], path + "/conditions/" + std::to_string(i), variables, scenes, report);
            }
        },
        [&](const AnyCondition& value) {
            for (std::size_t i = 0; i < value.conditions.size(); ++i) {
                if (value.conditions[i]) validate_condition(*value.conditions[i], path + "/conditions/" + std::to_string(i), variables, scenes, report);
            }
        },
        [&](const NotCondition& value) {
            if (value.condition) validate_condition(*value.condition, path + "/condition", variables, scenes, report);
        },
        [](const auto&) {}
    }, condition.data);
}

void validate_effect(const StoryEffect& effect, const std::string& path,
                     const std::map<std::string, StoryValue>& variables,
                     const std::set<std::string>& scenes,
                     const std::set<std::string>& chapters,
                     ValidationReport& report) {
    std::visit(Overloaded{
        [&](const ModifyVariableEffect& value) {
            const auto found = variables.find(value.variable.value());
            if (found == variables.end())
                report.add_error("semantic.missing_reference", path + "/variable",
                                 "unknown variable '" + value.variable.value() + "'");
            else if (found->second.index() != value.value.index())
                report.add_error("semantic.invalid_effect", path + "/value",
                                 "effect value must match the variable type");
            if (value.operation == VariableOperation::add &&
                (!std::holds_alternative<std::int64_t>(value.value) &&
                 !std::holds_alternative<double>(value.value)))
                report.add_error("semantic.invalid_effect", path + "/value",
                                 "add operation requires a numeric value");
        },
        [&](const SwitchSceneEffect& value) {
            if (!scenes.contains(value.scene.value()))
                report.add_error("semantic.missing_reference", path + "/scene",
                                 "unknown scene '" + value.scene.value() + "'");
        },
        [&](const AdvanceChapterEffect& value) {
            if (!chapters.contains(value.chapter.value()))
                report.add_error("semantic.missing_reference", path + "/chapter",
                                 "unknown chapter '" + value.chapter.value() + "'");
        },
        [&](const AddGoldEffect& value) {
            if (value.amount == 0) report.add_warning("semantic.no_op", path + "/amount", "zero gold change has no effect");
        },
        [](const auto&) {}
    }, effect.data);
}

} // namespace

bool ValidationReport::valid() const noexcept { return error_count() == 0; }

std::size_t ValidationReport::error_count() const noexcept {
    return static_cast<std::size_t>(std::count_if(issues.begin(), issues.end(),
        [](const auto& issue) { return issue.severity == IssueSeverity::error; }));
}

std::size_t ValidationReport::warning_count() const noexcept {
    return static_cast<std::size_t>(std::count_if(issues.begin(), issues.end(),
        [](const auto& issue) { return issue.severity == IssueSeverity::warning; }));
}

void ValidationReport::add_error(std::string code, std::string path, std::string message) {
    issues.push_back({IssueSeverity::error, std::move(code), std::move(path), std::move(message)});
}

void ValidationReport::add_warning(std::string code, std::string path, std::string message) {
    issues.push_back({IssueSeverity::warning, std::move(code), std::move(path), std::move(message)});
}

void ValidationReport::append(const ValidationReport& other) {
    issues.insert(issues.end(), other.issues.begin(), other.issues.end());
}

std::string to_string(IssueSeverity severity) {
    return severity == IssueSeverity::error ? "error" : "warning";
}

ValidationReport validate_story_package(const StoryPackage& package) {
    ValidationReport report;
    if (package.format != "storyforge.package/v1")
        report.add_error("semantic.unsupported_format", "/format",
                         "expected 'storyforge.package/v1'");
    if (package.chapters.empty())
        report.add_error("semantic.empty_package", "/chapters", "at least one chapter is required");

    std::map<std::string, StoryValue> variables;
    std::set<std::string> chapter_ids;
    std::set<std::string> scene_ids;
    std::map<std::string, Location> locations;

    for (std::size_t i = 0; i < package.variables.size(); ++i) {
        const auto& variable = package.variables[i];
        if (!variables.emplace(variable.id.value(), variable.initial_value).second)
            report.add_error("semantic.duplicate_id", "/variables/" + std::to_string(i) + "/id",
                             "duplicate variable id '" + variable.id.value() + "'");
    }

    for (std::size_t ci = 0; ci < package.chapters.size(); ++ci) {
        const auto& chapter = package.chapters[ci];
        const auto chapter_path = "/chapters/" + std::to_string(ci);
        require_unique(chapter_ids, chapter.id.value(), chapter_path + "/id", "chapter", report);
        std::set<std::string> chapter_scene_ids;
        for (std::size_t si = 0; si < chapter.scenes.size(); ++si) {
            const auto& scene = chapter.scenes[si];
            const auto scene_path = chapter_path + "/scenes/" + std::to_string(si);
            require_unique(chapter_scene_ids, scene.id.value(), scene_path + "/id", "scene in chapter", report);
            require_unique(scene_ids, scene.id.value(), scene_path + "/id", "scene", report);
            std::set<std::string> node_ids;
            for (std::size_t ni = 0; ni < scene.nodes.size(); ++ni) {
                const auto& node = scene.nodes[ni];
                const auto node_path = scene_path + "/nodes/" + std::to_string(ni);
                require_unique(node_ids, node.id.value(), node_path + "/id", "node in scene", report);
                locations.emplace(key(chapter.id, scene.id, node.id), Location{&chapter, &scene, &node, node_path});
                std::set<std::string> choice_ids;
                for (std::size_t hi = 0; hi < node.choices.size(); ++hi)
                    require_unique(choice_ids, node.choices[hi].id.value(),
                                   node_path + "/choices/" + std::to_string(hi) + "/id",
                                   "choice in node", report);
                if (node.terminal && !node.choices.empty())
                    report.add_error("semantic.terminal_has_choices", node_path + "/choices",
                                     "terminal node must not contain choices");
                if (!node.terminal && node.choices.empty() && package.agent_actions.empty() &&
                    package.agent_routes.empty() && package.agent_npcs.empty())
                    report.add_warning("semantic.dead_end", node_path,
                                       "non-terminal node has no choices");
            }
            if (!node_ids.contains(scene.entry_node.value()))
                report.add_error("semantic.missing_reference", scene_path + "/entry_node",
                                 "entry node '" + scene.entry_node.value() + "' is not in this scene");
        }
        if (!chapter_scene_ids.contains(chapter.entry_scene.value()))
            report.add_error("semantic.missing_reference", chapter_path + "/entry_scene",
                             "entry scene '" + chapter.entry_scene.value() + "' is not in this chapter");
    }

    for (const auto& [location_key, location] : locations) {
        (void)location_key;
        const auto& node = *location.node;
        for (std::size_t hi = 0; hi < node.choices.size(); ++hi) {
            const auto& choice = node.choices[hi];
            const auto choice_path = location.path + "/choices/" + std::to_string(hi);
            for (std::size_t i = 0; i < choice.conditions.size(); ++i)
                validate_condition(choice.conditions[i], choice_path + "/conditions/" + std::to_string(i), variables, scene_ids, report);
            for (std::size_t i = 0; i < choice.effects.size(); ++i)
                validate_effect(choice.effects[i], choice_path + "/effects/" + std::to_string(i), variables, scene_ids, chapter_ids, report);
            if (choice.target_node && find_node(*location.scene, *choice.target_node) == nullptr)
                report.add_error("semantic.missing_reference", choice_path + "/target_node",
                                 "target node '" + choice.target_node->value() + "' is not in this scene");
            const auto position_effects = std::count_if(choice.effects.begin(), choice.effects.end(), [](const StoryEffect& effect) {
                    return std::holds_alternative<SwitchSceneEffect>(effect.data) ||
                           std::holds_alternative<AdvanceChapterEffect>(effect.data);
                });
            if (position_effects + static_cast<int>(choice.target_node.has_value()) > 1)
                report.add_error("semantic.invalid_effect", choice_path,
                                 "a choice must not contain multiple position changes");
            const bool changes_position = choice.target_node.has_value() || position_effects > 0;
            if (!changes_position && !node.terminal)
                report.add_warning("semantic.choice_stays", choice_path,
                                   "choice has no target or position-changing effect");
        }
    }

    std::set<std::string> role_ids;
    for (const auto& [kind, entries] : package.world_lore) {
        if (kind != "facts" && kind != "rumors" && kind != "secrets" &&
            kind != "rules" && kind != "style")
            report.add_error("semantic.invalid_lore", "/world_lore/" + kind, "unknown lore category");
        if (entries.size() > 6144)
            report.add_error("semantic.invalid_lore", "/world_lore/" + kind, "too many entries");
        for (const auto& entry : entries)
            if (entry.empty() || entry.size() > 1000)
                report.add_error("semantic.invalid_lore", "/world_lore/" + kind, "entry must be 1-1000 bytes");
    }
    if (package.player_roles.size() > 12)
        report.add_error("semantic.invalid_role", "/player_roles", "too many player roles");
    for (std::size_t i = 0; i < package.player_roles.size(); ++i) {
        const auto& role = package.player_roles[i];
        const auto path = "/player_roles/" + std::to_string(i);
        require_unique(role_ids, role.id, path + "/id", "player role", report);
        if (role.name.empty() || role.name.size() > 120 || role.description.size() > 500 ||
            role.attributes.empty() || role.attributes.size() > 8 || role.starting_items.size() > 8)
            report.add_error("semantic.invalid_role", path, "role needs a name, 1-8 attributes, and at most 8 items");
        for (const auto& [name, value] : role.attributes)
            if (name.empty() || name.size() > 40 || value < -1000000 || value > 1000000)
                report.add_error("semantic.invalid_role", path + "/attributes", "attribute is out of range");
        for (const auto& [item, count] : role.starting_items)
            if (item.empty() || count < 1 || count > 10)
                report.add_error("semantic.invalid_role", path + "/starting_items", "item count must be 1-10");
        for (const auto& [item, name] : role.item_names)
            if (!role.starting_items.contains(item) || name.empty() || name.size() > 120)
                report.add_error("semantic.invalid_role", path + "/item_names", "item name must match a starting item");
        if ((role.fixed_name && (role.fixed_name->empty() || role.fixed_name->size() > 120)) ||
            role.starting_gold < 0 || role.starting_gold > 1000000 ||
            role.editable_fields.size() > 8 || role.traits.size() > 12 ||
            role.trait_count > role.traits.size())
            report.add_error("semantic.invalid_role", path, "invalid character creation options");
        std::set<std::string> labels;
        for (const auto& label : role.editable_fields)
            if (label.empty() || label.size() > 80 || !labels.insert(label).second)
                report.add_error("semantic.invalid_role", path + "/editable_fields", "field labels must be unique and short");
        labels.clear();
        for (const auto& label : role.traits)
            if (label.empty() || label.size() > 80 || !labels.insert(label).second)
                report.add_error("semantic.invalid_role", path + "/traits", "traits must be unique and short");
    }

    std::set<std::string> agent_action_ids;
    for (std::size_t i = 0; i < package.agent_actions.size(); ++i) {
        const auto& action = package.agent_actions[i];
        const auto path = "/agent_actions/" + std::to_string(i);
        require_unique(agent_action_ids, action.id.value(), path + "/id", "agent action", report);
        for (std::size_t j = 0; j < action.conditions.size(); ++j)
            validate_condition(action.conditions[j], path + "/conditions/" + std::to_string(j),
                               variables, scene_ids, report);
        for (std::size_t j = 0; j < action.effects.size(); ++j)
            validate_effect(action.effects[j], path + "/effects/" + std::to_string(j),
                            variables, scene_ids, chapter_ids, report);
        const auto position_effects = std::count_if(action.effects.begin(), action.effects.end(),
            [](const StoryEffect& effect) {
                return std::holds_alternative<SwitchSceneEffect>(effect.data) ||
                       std::holds_alternative<AdvanceChapterEffect>(effect.data);
            });
        if (position_effects + static_cast<int>(action.target_node.has_value()) > 1)
            report.add_error("semantic.invalid_effect", path, "an action must not contain multiple position changes");
        if (action.target_node) {
            bool found = false;
            for (const auto& chapter : package.chapters)
                for (const auto& scene : chapter.scenes)
                    found = found || find_node(scene, *action.target_node) != nullptr;
            if (!found)
                report.add_error("semantic.missing_reference", path + "/target_node",
                                 "agent action target does not exist in any scene");
        }
    }

    std::set<std::pair<std::string, std::string>> route_ids;
    for (std::size_t i = 0; i < package.agent_routes.size(); ++i) {
        const auto& route = package.agent_routes[i];
        const auto path = "/agent_routes/" + std::to_string(i);
        if (!scene_ids.contains(route.from.value()))
            report.add_error("semantic.missing_reference", path + "/from", "unknown source scene");
        if (!scene_ids.contains(route.to.value()))
            report.add_error("semantic.missing_reference", path + "/to", "unknown destination scene");
        if (route.from == route.to)
            report.add_error("semantic.invalid_route", path, "route cannot lead to the same scene");
        if (!route_ids.emplace(route.from.value(), route.to.value()).second)
            report.add_error("semantic.duplicate_id", path, "duplicate agent route");
        for (std::size_t j = 0; j < route.conditions.size(); ++j)
            validate_condition(route.conditions[j], path + "/conditions/" + std::to_string(j),
                               variables, scene_ids, report);
    }
    std::set<std::string> npc_ids;
    for (std::size_t i = 0; i < package.agent_npcs.size(); ++i) {
        const auto& npc = package.agent_npcs[i];
        const auto path = "/agent_npcs/" + std::to_string(i);
        require_unique(npc_ids, npc.id.value(), path + "/id", "agent NPC", report);
    }
    for (std::size_t i = 0; i < package.agent_npcs.size(); ++i) {
        const auto& npc = package.agent_npcs[i];
        const auto path = "/agent_npcs/" + std::to_string(i);
        if (!scene_ids.contains(npc.scene.value()))
            report.add_error("semantic.missing_reference", path + "/scene", "unknown NPC scene");
        if (npc.initial_relationship < -100 || npc.initial_relationship > 100)
            report.add_error("semantic.invalid_npc", path + "/initial_relationship",
                             "initial relationship must be between -100 and 100");
        std::set<std::string> topic_ids;
        for (std::size_t j = 0; j < npc.dialogue_rules.size(); ++j) {
            const auto& rule = npc.dialogue_rules[j];
            const auto rule_path = path + "/dialogue_rules/" + std::to_string(j);
            require_unique(topic_ids, rule.id, rule_path + "/id", "NPC dialogue topic", report);
            if (rule.id.empty() || rule.description.empty() || rule.reply.empty())
                report.add_error("semantic.invalid_dialogue", rule_path,
                                 "dialogue id, description, and reply must not be empty");
            for (std::size_t k = 0; k < rule.conditions.size(); ++k)
                validate_condition(rule.conditions[k], rule_path + "/conditions/" + std::to_string(k),
                                   variables, scene_ids, report);
            for (std::size_t k = 0; k < rule.effects.size(); ++k) {
                const auto& effect = rule.effects[k];
                const auto effect_path = rule_path + "/effects/" + std::to_string(k);
                validate_effect(effect, effect_path, variables, scene_ids, chapter_ids, report);
                if (const auto* memory = std::get_if<WriteNpcMemoryEffect>(&effect.data);
                    memory && !npc_ids.contains(memory->npc.value()))
                    report.add_error("semantic.missing_reference", effect_path + "/npc", "unknown NPC");
                if (const auto* relationship = std::get_if<ModifyNpcRelationshipEffect>(&effect.data);
                    relationship && !npc_ids.contains(relationship->npc.value()))
                    report.add_error("semantic.missing_reference", effect_path + "/npc", "unknown NPC");
                if (std::holds_alternative<SwitchSceneEffect>(effect.data) ||
                    std::holds_alternative<AdvanceChapterEffect>(effect.data))
                    report.add_error("semantic.invalid_dialogue", effect_path,
                                     "dialogue cannot change player position");
            }
        }
    }

    std::set<std::string> appointment_ids;
    for (std::size_t i = 0; i < package.agent_appointments.size(); ++i) {
        const auto& value = package.agent_appointments[i];
        const auto path = "/agent_appointments/" + std::to_string(i);
        require_unique(appointment_ids, value.id, path + "/id", "appointment", report);
        if (value.title.empty() || !npc_ids.contains(value.npc.value()) ||
            !scene_ids.contains(value.scene.value()) || value.start_minute < 1 ||
            value.start_minute > 10080 || value.duration_minutes < 1 ||
            value.duration_minutes > 240 || value.start_minute + value.duration_minutes > 10080)
            report.add_error("semantic.invalid_appointment", path,
                "appointment needs a valid NPC, scene, title and a 1-10080 minute time window");
    }

    std::set<std::string> object_ids;
    for (std::size_t i = 0; i < package.agent_objects.size(); ++i) {
        const auto& object = package.agent_objects[i];
        const auto path = "/agent_objects/" + std::to_string(i);
        require_unique(object_ids, object.id, path + "/id", "agent object", report);
        if (object.name.empty() || object.description.empty())
            report.add_error("semantic.invalid_object", path, "object name and description must not be empty");
        if (!scene_ids.contains(object.scene.value()))
            report.add_error("semantic.missing_reference", path + "/scene", "unknown object scene");
        if (object.item && object.item->empty())
            report.add_error("semantic.invalid_object", path + "/item", "item id must not be empty");
    }

    std::set<std::tuple<AgentActionKind, std::string, std::string>> item_rule_keys;
    for (std::size_t i = 0; i < package.agent_item_rules.size(); ++i) {
        const auto& rule = package.agent_item_rules[i];
        const auto path = "/agent_item_rules/" + std::to_string(i);
        if (rule.kind != AgentActionKind::give && rule.kind != AgentActionKind::use)
            report.add_error("semantic.invalid_item_rule", path + "/type", "expected give or use");
        if (rule.item.empty() || rule.description.empty())
            report.add_error("semantic.invalid_item_rule", path, "item and description must not be empty");
        if (rule.kind == AgentActionKind::give && !npc_ids.contains(rule.target))
            report.add_error("semantic.missing_reference", path + "/target", "unknown NPC target");
        if (rule.kind == AgentActionKind::use && !object_ids.contains(rule.target))
            report.add_error("semantic.missing_reference", path + "/target", "unknown object target");
        if (!item_rule_keys.emplace(rule.kind, rule.item.value(), rule.target).second)
            report.add_error("semantic.duplicate_id", path, "duplicate item interaction");
        if (rule.kind == AgentActionKind::give && rule.consume)
            report.add_error("semantic.invalid_item_rule", path + "/consume", "give always transfers one item");
        for (std::size_t j = 0; j < rule.conditions.size(); ++j)
            validate_condition(rule.conditions[j], path + "/conditions/" + std::to_string(j),
                               variables, scene_ids, report);
        for (std::size_t j = 0; j < rule.effects.size(); ++j) {
            const auto effect_path = path + "/effects/" + std::to_string(j);
            const auto& effect = rule.effects[j];
            validate_effect(effect, effect_path, variables, scene_ids, chapter_ids, report);
            if (std::holds_alternative<SwitchSceneEffect>(effect.data) ||
                std::holds_alternative<AdvanceChapterEffect>(effect.data))
                report.add_error("semantic.invalid_item_rule", effect_path,
                                 "item interaction cannot change player position");
        }
    }

    const StoryChapter* entry_chapter = find_chapter(package, package.entry.chapter);
    const StoryScene* entry_scene = entry_chapter == nullptr ? nullptr : find_scene(*entry_chapter, package.entry.scene);
    const StoryNode* entry_node = entry_scene == nullptr ? nullptr : find_node(*entry_scene, package.entry.node);
    if (entry_chapter == nullptr)
        report.add_error("semantic.invalid_entry", "/entry/chapter", "entry chapter does not exist");
    else if (entry_scene == nullptr)
        report.add_error("semantic.invalid_entry", "/entry/scene", "entry scene is not in the entry chapter");
    else if (entry_node == nullptr)
        report.add_error("semantic.invalid_entry", "/entry/node", "entry node is not in the entry scene");

    if (entry_node != nullptr) {
        std::set<std::string> reachable;
        std::deque<std::string> pending;
        pending.push_back(key(entry_chapter->id, entry_scene->id, entry_node->id));
        while (!pending.empty()) {
            const auto current_key = pending.front();
            pending.pop_front();
            if (!reachable.insert(current_key).second) continue;
            const auto found = locations.find(current_key);
            if (found == locations.end()) continue;
            const auto& location = found->second;
            for (const auto& choice : location.node->choices) {
                if (choice.target_node) pending.push_back(key(location.chapter->id, location.scene->id, *choice.target_node));
                for (const auto& effect : choice.effects) {
                    std::visit(Overloaded{
                        [&](const SwitchSceneEffect& value) {
                            for (const auto& chapter : package.chapters) {
                                if (const auto* scene = find_scene(chapter, value.scene))
                                    pending.push_back(key(chapter.id, scene->id, scene->entry_node));
                            }
                        },
                        [&](const AdvanceChapterEffect& value) {
                            if (const auto* chapter = find_chapter(package, value.chapter)) {
                                if (const auto* scene = find_scene(*chapter, chapter->entry_scene))
                                    pending.push_back(key(chapter->id, scene->id, scene->entry_node));
                            }
                        },
                        [](const auto&) {}
                    }, effect.data);
                }
            }
            if (location.node->terminal) continue;
            for (const auto& route : package.agent_routes) {
                if (route.from != location.scene->id) continue;
                for (const auto& chapter : package.chapters) {
                    if (const auto* destination = find_scene(chapter, route.to))
                        pending.push_back(key(chapter.id, destination->id, destination->entry_node));
                }
            }
            for (const auto& action : package.agent_actions) {
                if (action.target_node && find_node(*location.scene, *action.target_node))
                    pending.push_back(key(location.chapter->id, location.scene->id, *action.target_node));
                for (const auto& effect : action.effects) {
                    std::visit(Overloaded{
                        [&](const SwitchSceneEffect& value) {
                            for (const auto& chapter : package.chapters) {
                                if (const auto* scene = find_scene(chapter, value.scene))
                                    pending.push_back(key(chapter.id, scene->id, scene->entry_node));
                            }
                        },
                        [&](const AdvanceChapterEffect& value) {
                            if (const auto* chapter = find_chapter(package, value.chapter)) {
                                if (const auto* scene = find_scene(*chapter, chapter->entry_scene))
                                    pending.push_back(key(chapter->id, scene->id, scene->entry_node));
                            }
                        },
                        [](const auto&) {}
                    }, effect.data);
                }
            }
        }
        for (const auto& [node_key, location] : locations) {
            if (!reachable.contains(node_key))
                report.add_error("semantic.unreachable_node", location.path,
                                 "node '" + location.node->id.value() + "' is unreachable from the package entry");
        }
    }
    return report;
}

} // namespace storyforge
