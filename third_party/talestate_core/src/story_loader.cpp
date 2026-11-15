#include "storyforge/io/story_loader.hpp"

#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <regex>
#include <set>
#include <system_error>

#include <nlohmann/json.hpp>

namespace storyforge {
namespace {

using Json = nlohmann::json;
constexpr std::uintmax_t max_package_bytes = 16U * 1024U * 1024U;

void type_error(ValidationReport& report, const std::string& path, const std::string& expected) {
    report.add_error("schema.type", path, "expected " + expected);
}

void reject_unknown_properties(const Json& object, const std::string& path,
                               std::initializer_list<const char*> allowed,
                               ValidationReport& report) {
    if (!object.is_object()) return;
    std::set<std::string> names;
    for (const char* name : allowed) names.emplace(name);
    for (const auto& [name, value] : object.items()) {
        (void)value;
        if (!names.contains(name))
            report.add_error("schema.additionalProperties", path + "/" + name,
                             "unknown property is not allowed");
    }
}

const Json* member(const Json& object, const char* name, const std::string& path,
                   ValidationReport& report) {
    if (!object.is_object()) {
        type_error(report, path, "object");
        return nullptr;
    }
    const auto it = object.find(name);
    if (it == object.end()) {
        report.add_error("schema.required", path + "/" + name, "required property is missing");
        return nullptr;
    }
    return &*it;
}

std::string read_string(const Json& object, const char* name, const std::string& path,
                        ValidationReport& report) {
    const Json* value = member(object, name, path, report);
    if (value == nullptr) return {};
    if (!value->is_string()) {
        type_error(report, path + "/" + name, "string");
        return {};
    }
    const auto result = value->get<std::string>();
    if (result.empty()) report.add_error("schema.minLength", path + "/" + name, "must not be empty");
    return result;
}

std::string read_id(const Json& object, const char* name, const std::string& path,
                    ValidationReport& report) {
    auto value = read_string(object, name, path, report);
    static const std::regex pattern{"^[a-z][a-z0-9_.-]*$"};
    if (!value.empty() && !std::regex_match(value, pattern))
        report.add_error("schema.pattern", path + "/" + name,
                         "id must match ^[a-z][a-z0-9_.-]*$");
    return value;
}

std::int64_t read_integer(const Json& object, const char* name, const std::string& path,
                          ValidationReport& report, std::int64_t fallback = 0) {
    const Json* value = member(object, name, path, report);
    if (value == nullptr) return fallback;
    if (!value->is_number_integer()) {
        type_error(report, path + "/" + name, "integer");
        return fallback;
    }
    return value->get<std::int64_t>();
}

const Json* read_array(const Json& object, const char* name, const std::string& path,
                       ValidationReport& report) {
    const Json* value = member(object, name, path, report);
    if (value != nullptr && !value->is_array()) {
        type_error(report, path + "/" + name, "array");
        return nullptr;
    }
    return value;
}

std::optional<StoryValue> read_value(const Json& value, const std::string& path,
                                     ValidationReport& report) {
    if (value.is_boolean()) return StoryValue{value.get<bool>()};
    if (value.is_number_integer()) return StoryValue{value.get<std::int64_t>()};
    if (value.is_number_float()) return StoryValue{value.get<double>()};
    if (value.is_string()) return StoryValue{value.get<std::string>()};
    type_error(report, path, "string, boolean, integer, or number");
    return std::nullopt;
}

std::optional<CompareOperator> read_compare_operator(const std::string& value,
                                                     const std::string& path,
                                                     ValidationReport& report) {
    if (value == "eq") return CompareOperator::equal;
    if (value == "ne") return CompareOperator::not_equal;
    if (value == "lt") return CompareOperator::less;
    if (value == "lte") return CompareOperator::less_equal;
    if (value == "gt") return CompareOperator::greater;
    if (value == "gte") return CompareOperator::greater_equal;
    report.add_error("schema.enum", path, "expected one of: eq, ne, lt, lte, gt, gte");
    return std::nullopt;
}

std::optional<StoryCondition> read_condition(const Json& json, const std::string& path,
                                             ValidationReport& report) {
    if (!json.is_object()) {
        type_error(report, path, "object");
        return std::nullopt;
    }
    const auto type = read_string(json, "type", path, report);
    if (type == "variable") {
        reject_unknown_properties(json, path, {"type", "variable", "op", "value"}, report);
        const auto variable = read_id(json, "variable", path, report);
        const auto op_text = read_string(json, "op", path, report);
        const Json* raw_value = member(json, "value", path, report);
        auto op = read_compare_operator(op_text, path + "/op", report);
        auto value = raw_value == nullptr ? std::nullopt : read_value(*raw_value, path + "/value", report);
        if (op && value) return StoryCondition{VariableComparison{VariableId{variable}, *op, *value}};
    } else if (type == "has_item") {
        reject_unknown_properties(json, path, {"type", "item", "quantity"}, report);
        const auto item = read_id(json, "item", path, report);
        const auto quantity = json.contains("quantity")
            ? read_integer(json, "quantity", path, report, 1) : 1;
        if (quantity < 1) report.add_error("schema.minimum", path + "/quantity", "must be at least 1");
        return StoryCondition{HasItemCondition{ItemId{item}, quantity}};
    } else if (type == "gold") {
        reject_unknown_properties(json, path, {"type", "op", "amount"}, report);
        const auto op_text = read_string(json, "op", path, report);
        const auto amount = read_integer(json, "amount", path, report);
        auto op = read_compare_operator(op_text, path + "/op", report);
        if (op) return StoryCondition{GoldCondition{*op, amount}};
    } else if (type == "current_scene") {
        reject_unknown_properties(json, path, {"type", "scene"}, report);
        return StoryCondition{CurrentSceneCondition{SceneId{read_id(json, "scene", path, report)}}};
    } else if (type == "event_triggered") {
        reject_unknown_properties(json, path, {"type", "event"}, report);
        return StoryCondition{EventTriggeredCondition{EventId{read_id(json, "event", path, report)}}};
    } else if (type == "npc_memory") {
        reject_unknown_properties(json, path, {"type", "npc", "key", "value"}, report);
        const auto npc = read_id(json, "npc", path, report);
        const auto key = read_string(json, "key", path, report);
        const Json* raw_value = member(json, "value", path, report);
        auto value = raw_value == nullptr ? std::nullopt : read_value(*raw_value, path + "/value", report);
        if (value) return StoryCondition{NpcMemoryCondition{NpcId{npc}, key, *value}};
    } else if (type == "npc_relationship") {
        reject_unknown_properties(json, path, {"type", "npc", "op", "value"}, report);
        const auto npc = NpcId{read_id(json, "npc", path, report)};
        const auto op_text = read_string(json, "op", path, report);
        const auto value = read_integer(json, "value", path, report);
        if (auto op = read_compare_operator(op_text, path + "/op", report))
            return StoryCondition{NpcRelationshipCondition{npc, *op, value}};
    } else if (type == "all" || type == "any") {
        reject_unknown_properties(json, path, {"type", "conditions"}, report);
        const Json* raw_conditions = read_array(json, "conditions", path, report);
        std::vector<ConditionPtr> conditions;
        if (raw_conditions != nullptr) {
            for (std::size_t i = 0; i < raw_conditions->size(); ++i) {
                auto condition = read_condition((*raw_conditions)[i], path + "/conditions/" + std::to_string(i), report);
                if (condition) conditions.push_back(std::make_shared<StoryCondition>(std::move(*condition)));
            }
            if (conditions.empty()) report.add_error("schema.minItems", path + "/conditions", "must contain at least one condition");
        }
        if (type == "all") return StoryCondition{AllCondition{std::move(conditions)}};
        return StoryCondition{AnyCondition{std::move(conditions)}};
    } else if (type == "not") {
        reject_unknown_properties(json, path, {"type", "condition"}, report);
        const Json* raw_condition = member(json, "condition", path, report);
        if (raw_condition != nullptr) {
            auto condition = read_condition(*raw_condition, path + "/condition", report);
            if (condition) return StoryCondition{NotCondition{std::make_shared<StoryCondition>(std::move(*condition))}};
        }
    } else if (!type.empty()) {
        report.add_error("schema.enum", path + "/type", "unknown condition type '" + type + "'");
    }
    return std::nullopt;
}

std::optional<StoryEffect> read_effect(const Json& json, const std::string& path,
                                       ValidationReport& report) {
    if (!json.is_object()) {
        type_error(report, path, "object");
        return std::nullopt;
    }
    const auto type = read_string(json, "type", path, report);
    if (type == "modify_variable") {
        reject_unknown_properties(json, path, {"type", "variable", "operation", "value"}, report);
        const auto variable = read_id(json, "variable", path, report);
        const auto operation_text = read_string(json, "operation", path, report);
        VariableOperation operation{VariableOperation::set};
        if (operation_text == "add") operation = VariableOperation::add;
        else if (operation_text != "set") report.add_error("schema.enum", path + "/operation", "expected set or add");
        const Json* raw_value = member(json, "value", path, report);
        auto value = raw_value == nullptr ? std::nullopt : read_value(*raw_value, path + "/value", report);
        if (value) return StoryEffect{ModifyVariableEffect{VariableId{variable}, operation, *value}};
    } else if (type == "add_gold") {
        reject_unknown_properties(json, path, {"type", "amount"}, report);
        return StoryEffect{AddGoldEffect{read_integer(json, "amount", path, report)}};
    } else if (type == "add_item" || type == "remove_item") {
        reject_unknown_properties(json, path, {"type", "item", "quantity"}, report);
        const auto item = ItemId{read_id(json, "item", path, report)};
        const auto quantity = json.contains("quantity") ? read_integer(json, "quantity", path, report, 1) : 1;
        if (quantity < 1) report.add_error("schema.minimum", path + "/quantity", "must be at least 1");
        if (type == "add_item") return StoryEffect{AddItemEffect{item, quantity}};
        return StoryEffect{RemoveItemEffect{item, quantity}};
    } else if (type == "switch_scene") {
        reject_unknown_properties(json, path, {"type", "scene"}, report);
        return StoryEffect{SwitchSceneEffect{SceneId{read_id(json, "scene", path, report)}}};
    } else if (type == "advance_chapter") {
        reject_unknown_properties(json, path, {"type", "chapter"}, report);
        return StoryEffect{AdvanceChapterEffect{ChapterId{read_id(json, "chapter", path, report)}}};
    } else if (type == "write_npc_memory") {
        reject_unknown_properties(json, path, {"type", "npc", "key", "value"}, report);
        const auto npc = NpcId{read_id(json, "npc", path, report)};
        const auto key = read_string(json, "key", path, report);
        const Json* raw_value = member(json, "value", path, report);
        auto value = raw_value == nullptr ? std::nullopt : read_value(*raw_value, path + "/value", report);
        if (value) return StoryEffect{WriteNpcMemoryEffect{npc, key, *value}};
    } else if (type == "modify_npc_relationship") {
        reject_unknown_properties(json, path, {"type", "npc", "amount"}, report);
        const auto npc = NpcId{read_id(json, "npc", path, report)};
        const auto amount = read_integer(json, "amount", path, report);
        return StoryEffect{ModifyNpcRelationshipEffect{npc, amount}};
    } else if (type == "complete_event") {
        reject_unknown_properties(json, path, {"type", "event"}, report);
        return StoryEffect{CompleteEventEffect{EventId{read_id(json, "event", path, report)}}};
    } else if (type == "append_narrative") {
        reject_unknown_properties(json, path, {"type", "text"}, report);
        return StoryEffect{AppendNarrativeEffect{read_string(json, "text", path, report)}};
    } else if (!type.empty()) {
        report.add_error("schema.enum", path + "/type", "unknown effect type '" + type + "'");
    }
    return std::nullopt;
}

StoryChoice read_choice(const Json& json, const std::string& path, ValidationReport& report) {
    reject_unknown_properties(json, path, {"id", "text", "conditions", "effects", "target_node"}, report);
    StoryChoice choice{ChoiceId{read_id(json, "id", path, report)},
                       read_string(json, "text", path, report), {}, {}, std::nullopt};
    if (const Json* values = read_array(json, "conditions", path, report)) {
        for (std::size_t i = 0; i < values->size(); ++i) {
            auto value = read_condition((*values)[i], path + "/conditions/" + std::to_string(i), report);
            if (value) choice.conditions.push_back(std::move(*value));
        }
    }
    if (const Json* values = read_array(json, "effects", path, report)) {
        for (std::size_t i = 0; i < values->size(); ++i) {
            auto value = read_effect((*values)[i], path + "/effects/" + std::to_string(i), report);
            if (value) choice.effects.push_back(std::move(*value));
        }
    }
    if (const auto it = json.find("target_node"); it != json.end()) {
        if (it->is_string()) {
            const auto target = read_id(json, "target_node", path, report);
            choice.target_node = NodeId{target};
        }
        else type_error(report, path + "/target_node", "string");
    }
    return choice;
}

StoryNode read_node(const Json& json, const std::string& path, ValidationReport& report) {
    reject_unknown_properties(json, path, {"id", "text", "terminal", "choices"}, report);
    StoryNode node{NodeId{read_id(json, "id", path, report)},
                   read_string(json, "text", path, report), {}, false};
    if (const auto it = json.find("terminal"); it != json.end()) {
        if (it->is_boolean()) node.terminal = it->get<bool>();
        else type_error(report, path + "/terminal", "boolean");
    }
    if (const Json* values = read_array(json, "choices", path, report)) {
        for (std::size_t i = 0; i < values->size(); ++i)
            node.choices.push_back(read_choice((*values)[i], path + "/choices/" + std::to_string(i), report));
    }
    return node;
}

StoryScene read_scene(const Json& json, const std::string& path, ValidationReport& report) {
    reject_unknown_properties(json, path, {"id", "title", "entry_node", "nodes"}, report);
    StoryScene scene{SceneId{read_id(json, "id", path, report)},
                     read_string(json, "title", path, report),
                     NodeId{read_id(json, "entry_node", path, report)}, {}};
    if (const Json* values = read_array(json, "nodes", path, report)) {
        for (std::size_t i = 0; i < values->size(); ++i)
            scene.nodes.push_back(read_node((*values)[i], path + "/nodes/" + std::to_string(i), report));
    }
    return scene;
}

StoryChapter read_chapter(const Json& json, const std::string& path, ValidationReport& report) {
    reject_unknown_properties(json, path, {"id", "title", "entry_scene", "scenes"}, report);
    StoryChapter chapter{ChapterId{read_id(json, "id", path, report)},
                         read_string(json, "title", path, report),
                         SceneId{read_id(json, "entry_scene", path, report)}, {}};
    if (const Json* values = read_array(json, "scenes", path, report)) {
        for (std::size_t i = 0; i < values->size(); ++i)
            chapter.scenes.push_back(read_scene((*values)[i], path + "/scenes/" + std::to_string(i), report));
    }
    return chapter;
}

StoryPackage read_package(const Json& json, ValidationReport& report) {
    if (!json.is_object()) {
        type_error(report, "", "object");
        return {};
    }
    StoryPackage package;
    reject_unknown_properties(json, "", {"format", "id", "title", "version", "entry", "variables", "chapters", "world_context", "world_lore", "agent_actions", "agent_routes", "agent_npcs", "agent_objects", "agent_item_rules", "agent_appointments", "player_roles"}, report);
    package.format = read_string(json, "format", "", report);
    package.id = StoryId{read_id(json, "id", "", report)};
    package.title = read_string(json, "title", "", report);
    package.version = read_string(json, "version", "", report);
    static const std::regex semver{"^[0-9]+\\.[0-9]+\\.[0-9]+$"};
    if (!package.version.empty() && !std::regex_match(package.version, semver))
        report.add_error("schema.pattern", "/version", "version must use MAJOR.MINOR.PATCH");

    if (const Json* entry = member(json, "entry", "", report)) {
        reject_unknown_properties(*entry, "/entry", {"chapter", "scene", "node"}, report);
        package.entry.chapter = ChapterId{read_id(*entry, "chapter", "/entry", report)};
        package.entry.scene = SceneId{read_id(*entry, "scene", "/entry", report)};
        package.entry.node = NodeId{read_id(*entry, "node", "/entry", report)};
    }
    if (const Json* variables = read_array(json, "variables", "", report)) {
        for (std::size_t i = 0; i < variables->size(); ++i) {
            const auto path = "/variables/" + std::to_string(i);
            const auto& raw = (*variables)[i];
            reject_unknown_properties(raw, path, {"id", "initial"}, report);
            const auto id = VariableId{read_id(raw, "id", path, report)};
            const Json* initial = member(raw, "initial", path, report);
            auto value = initial == nullptr ? std::nullopt : read_value(*initial, path + "/initial", report);
            if (value) package.variables.push_back(StoryVariable{id, *value});
        }
    }
    if (const Json* chapters = read_array(json, "chapters", "", report)) {
        for (std::size_t i = 0; i < chapters->size(); ++i)
            package.chapters.push_back(read_chapter((*chapters)[i], "/chapters/" + std::to_string(i), report));
    }
    if (const auto it = json.find("world_context"); it != json.end()) {
        if (it->is_string()) package.world_context = it->get<std::string>();
        else type_error(report, "/world_context", "string");
    }
    if (const auto it = json.find("world_lore"); it != json.end()) {
        if (!it->is_object()) type_error(report, "/world_lore", "object");
        else for (const auto& [key, value] : it->items()) {
            if (!value.is_array()) { type_error(report, "/world_lore/" + key, "array"); continue; }
            for (std::size_t i = 0; i < value.size(); ++i) {
                if (!value[i].is_string()) type_error(report, "/world_lore/" + key + "/" + std::to_string(i), "string");
                else package.world_lore[key].push_back(value[i].get<std::string>());
            }
        }
    }
    if (const auto it = json.find("player_roles"); it != json.end()) {
        if (!it->is_array()) type_error(report, "/player_roles", "array");
        else for (std::size_t i = 0; i < it->size(); ++i) {
            const auto path = "/player_roles/" + std::to_string(i);
            const auto& raw = (*it)[i];
            reject_unknown_properties(raw, path, {"id", "name", "description", "attributes", "starting_items", "item_names", "fixed_name", "starting_gold", "editable_fields", "traits", "trait_count"}, report);
            PlayerRole role;
            role.id = read_id(raw, "id", path, report);
            role.name = read_string(raw, "name", path, report);
            role.description = read_string(raw, "description", path, report);
            if (const auto value = raw.find("fixed_name"); value != raw.end()) {
                if (!value->is_string()) type_error(report, path + "/fixed_name", "string");
                else role.fixed_name = value->get<std::string>();
            }
            if (const auto value = raw.find("starting_gold"); value != raw.end()) {
                if (!value->is_number_integer()) type_error(report, path + "/starting_gold", "integer");
                else role.starting_gold = value->get<std::int64_t>();
            }
            if (const auto value = raw.find("trait_count"); value != raw.end()) {
                if (!value->is_number_unsigned()) type_error(report, path + "/trait_count", "unsigned integer");
                else role.trait_count = value->get<std::size_t>();
            }
            for (const auto* field : {"editable_fields", "traits"}) {
                const auto value = raw.find(field);
                if (value == raw.end()) continue;
                if (!value->is_array()) { type_error(report, path + "/" + field, "array"); continue; }
                for (std::size_t j = 0; j < value->size(); ++j) {
                    if (!(*value)[j].is_string()) type_error(report, path + "/" + field + "/" + std::to_string(j), "string");
                    else if (std::string_view(field) == "traits") role.traits.push_back((*value)[j].get<std::string>());
                    else role.editable_fields.push_back((*value)[j].get<std::string>());
                }
            }
            for (const auto* field : {"attributes", "starting_items"}) {
                const auto values = raw.is_object() ? raw.find(field) : raw.end();
                if (values == raw.end() || !values->is_object()) {
                    type_error(report, path + "/" + field, "object");
                    continue;
                }
                for (const auto& [key, value] : values->items()) {
                    if (!value.is_number_integer()) { type_error(report, path + "/" + field + "/" + key, "integer"); continue; }
                    const auto number = value.get<std::int64_t>();
                    if (std::string_view(field) == "attributes") role.attributes[key] = number;
                    else role.starting_items[ItemId{key}] = number;
                }
            }
            if (const auto names = raw.find("item_names"); names != raw.end()) {
                if (!names->is_object()) type_error(report, path + "/item_names", "object");
                else for (const auto& [key, value] : names->items()) {
                    if (!value.is_string()) type_error(report, path + "/item_names/" + key, "string");
                    else role.item_names[ItemId{key}] = value.get<std::string>();
                }
            }
            package.player_roles.push_back(std::move(role));
        }
    }
    if (const auto it = json.find("agent_actions"); it != json.end()) {
        if (!it->is_array()) type_error(report, "/agent_actions", "array");
        else for (std::size_t i = 0; i < it->size(); ++i) {
            auto choice = read_choice((*it)[i], "/agent_actions/" + std::to_string(i), report);
            package.agent_actions.push_back({std::move(choice.id), std::move(choice.text),
                std::move(choice.conditions), std::move(choice.effects), std::move(choice.target_node)});
        }
    }
    if (const auto it = json.find("agent_routes"); it != json.end()) {
        if (!it->is_array()) type_error(report, "/agent_routes", "array");
        else for (std::size_t i = 0; i < it->size(); ++i) {
            const auto path = "/agent_routes/" + std::to_string(i);
            const auto& raw = (*it)[i];
            reject_unknown_properties(raw, path, {"from", "to", "conditions"}, report);
            AgentRoute route{SceneId{read_id(raw, "from", path, report)},
                             SceneId{read_id(raw, "to", path, report)}, {}};
            if (const Json* values = read_array(raw, "conditions", path, report)) {
                for (std::size_t j = 0; j < values->size(); ++j) {
                    auto condition = read_condition((*values)[j], path + "/conditions/" + std::to_string(j), report);
                    if (condition) route.conditions.push_back(std::move(*condition));
                }
            }
            package.agent_routes.push_back(std::move(route));
        }
    }
    if (const auto it = json.find("agent_npcs"); it != json.end()) {
        if (!it->is_array()) type_error(report, "/agent_npcs", "array");
        else for (std::size_t i = 0; i < it->size(); ++i) {
            const auto path = "/agent_npcs/" + std::to_string(i);
            const auto& raw = (*it)[i];
            reject_unknown_properties(raw, path, {"id", "name", "scene", "dialogue_rules", "initial_relationship"}, report);
            AgentNpc npc{NpcId{read_id(raw, "id", path, report)},
                read_string(raw, "name", path, report), SceneId{read_id(raw, "scene", path, report)}, {}};
            if (const auto value = raw.is_object() ? raw.find("initial_relationship") : raw.end();
                value != raw.end()) {
                if (value->is_number_integer()) npc.initial_relationship = value->get<std::int64_t>();
                else type_error(report, path + "/initial_relationship", "integer");
            }
            if (const auto rules = raw.is_object() ? raw.find("dialogue_rules") : raw.end();
                rules != raw.end()) {
                if (!rules->is_array()) type_error(report, path + "/dialogue_rules", "array");
                else for (std::size_t j = 0; j < rules->size(); ++j) {
                    const auto rule_path = path + "/dialogue_rules/" + std::to_string(j);
                    const auto& rule_raw = (*rules)[j];
                    reject_unknown_properties(rule_raw, rule_path,
                        {"id", "description", "reply", "conditions", "effects"}, report);
                    AgentDialogueRule rule{read_id(rule_raw, "id", rule_path, report),
                        read_string(rule_raw, "description", rule_path, report),
                        read_string(rule_raw, "reply", rule_path, report), {}, {}};
                    if (const Json* values = read_array(rule_raw, "conditions", rule_path, report)) {
                        for (std::size_t k = 0; k < values->size(); ++k) {
                            auto condition = read_condition((*values)[k], rule_path + "/conditions/" + std::to_string(k), report);
                            if (condition) rule.conditions.push_back(std::move(*condition));
                        }
                    }
                    if (const Json* values = read_array(rule_raw, "effects", rule_path, report)) {
                        for (std::size_t k = 0; k < values->size(); ++k) {
                            auto effect = read_effect((*values)[k], rule_path + "/effects/" + std::to_string(k), report);
                            if (effect) rule.effects.push_back(std::move(*effect));
                        }
                    }
                    npc.dialogue_rules.push_back(std::move(rule));
                }
            }
            package.agent_npcs.push_back(std::move(npc));
        }
    }
    if (const auto it = json.find("agent_appointments"); it != json.end()) {
        if (!it->is_array()) type_error(report, "/agent_appointments", "array");
        else for (std::size_t i = 0; i < it->size(); ++i) {
            const auto path = "/agent_appointments/" + std::to_string(i);
            const auto& raw = (*it)[i];
            reject_unknown_properties(raw, path,
                {"id", "title", "npc", "scene", "start_minute", "duration_minutes"}, report);
            const auto start = read_integer(raw, "start_minute", path, report);
            const auto duration = read_integer(raw, "duration_minutes", path, report);
            package.agent_appointments.push_back({read_id(raw, "id", path, report),
                read_string(raw, "title", path, report), NpcId{read_id(raw, "npc", path, report)},
                SceneId{read_id(raw, "scene", path, report)},
                start < 0 ? 0U : static_cast<std::uint64_t>(start),
                duration < 0 ? 0U : static_cast<std::uint64_t>(duration)});
        }
    }
    if (const auto it = json.find("agent_objects"); it != json.end()) {
        if (!it->is_array()) type_error(report, "/agent_objects", "array");
        else for (std::size_t i = 0; i < it->size(); ++i) {
            const auto path = "/agent_objects/" + std::to_string(i);
            const auto& raw = (*it)[i];
            reject_unknown_properties(raw, path, {"id", "name", "scene", "description", "item"}, report);
            AgentObject object{read_id(raw, "id", path, report),
                read_string(raw, "name", path, report),
                SceneId{read_id(raw, "scene", path, report)},
                read_string(raw, "description", path, report), std::nullopt};
            if (raw.is_object() && raw.contains("item"))
                object.item = ItemId{read_id(raw, "item", path, report)};
            package.agent_objects.push_back(std::move(object));
        }
    }
    if (const auto it = json.find("agent_item_rules"); it != json.end()) {
        if (!it->is_array()) type_error(report, "/agent_item_rules", "array");
        else for (std::size_t i = 0; i < it->size(); ++i) {
            const auto path = "/agent_item_rules/" + std::to_string(i);
            const auto& raw = (*it)[i];
            reject_unknown_properties(raw, path,
                {"type", "item", "target", "description", "consume", "conditions", "effects"}, report);
            const auto type = read_string(raw, "type", path, report);
            if (type != "give" && type != "use")
                report.add_error("schema.enum", path + "/type", "expected give or use");
            AgentItemRule rule{type == "give" ? AgentActionKind::give : AgentActionKind::use,
                ItemId{read_id(raw, "item", path, report)},
                read_id(raw, "target", path, report),
                read_string(raw, "description", path, report), false, {}, {}};
            if (raw.is_object() && raw.contains("consume")) {
                if (raw["consume"].is_boolean()) rule.consume = raw["consume"].get<bool>();
                else type_error(report, path + "/consume", "boolean");
            }
            if (const Json* values = read_array(raw, "conditions", path, report))
                for (std::size_t j = 0; j < values->size(); ++j) {
                    auto condition = read_condition((*values)[j], path + "/conditions/" + std::to_string(j), report);
                    if (condition) rule.conditions.push_back(std::move(*condition));
                }
            if (const Json* values = read_array(raw, "effects", path, report))
                for (std::size_t j = 0; j < values->size(); ++j) {
                    auto effect = read_effect((*values)[j], path + "/effects/" + std::to_string(j), report);
                    if (effect) rule.effects.push_back(std::move(*effect));
                }
            package.agent_item_rules.push_back(std::move(rule));
        }
    }
    return package;
}

} // namespace

LoadResult load_story_package_text(const std::string& json_text) {
    LoadResult result;
    if (json_text.size() > max_package_bytes) {
        result.report.add_error("io.too_large", "", "story package exceeds the 16 MiB limit");
        return result;
    }
    try {
        const Json json = Json::parse(json_text, nullptr, true, true);
        auto package = read_package(json, result.report);
        if (result.report.valid()) {
            result.package = std::move(package);
            result.report.append(validate_story_package(*result.package));
        }
    } catch (const Json::parse_error& exception) {
        result.report.add_error("json.parse", "", exception.what());
    } catch (const Json::exception& exception) {
        result.report.add_error("json.value", "", exception.what());
    }
    return result;
}

LoadResult load_story_package(const std::filesystem::path& requested_path) {
    LoadResult result;
    std::filesystem::path path = requested_path;
    std::error_code error;
    if (std::filesystem::is_directory(path, error)) path /= "story.json";
    if (error || !std::filesystem::is_regular_file(path, error)) {
        result.report.add_error("io.not_found", path.string(), "story package file does not exist");
        return result;
    }
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > max_package_bytes) {
        result.report.add_error("io.too_large", path.string(), "story package exceeds the 16 MiB limit");
        return result;
    }
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        result.report.add_error("io.unreadable", path.string(), "story package cannot be opened");
        return result;
    }
    const std::string content{std::istreambuf_iterator<char>{stream},
                              std::istreambuf_iterator<char>{}};
    if (stream.bad()) {
        result.report.add_error("io.unreadable", path.string(), "story package cannot be read");
        return result;
    }
    return load_story_package_text(content);
}

} // namespace storyforge
