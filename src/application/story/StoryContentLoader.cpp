#include "application/story/StoryContentLoader.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <limits>

namespace arcane::application::story {

namespace {

LocalizedStoryText localizedText(const QJsonObject& parent, const char* key)
{
    const auto value = parent.value(QLatin1String(key)).toObject();
    return {value.value(QStringLiteral("zh")).toString(),
            value.value(QStringLiteral("en")).toString()};
}

QStringList stringList(const QJsonValue& value)
{
    QStringList result;
    for (const auto& entry : value.toArray()) result.push_back(entry.toString());
    return result;
}

std::optional<StoryCondition> parseCondition(const QJsonObject& object, QString& error)
{
    const QString type = object.value(QStringLiteral("type")).toString();
    if (type == QLatin1String("has_flag") || type == QLatin1String("missing_flag")) {
        return FlagCondition{object.value(QStringLiteral("key")).toString(),
                             type == QLatin1String("has_flag")};
    }
    if (type == QLatin1String("has_item")) {
        const auto itemId = object.value(QStringLiteral("item_id")).toInteger();
        const auto minimum = object.value(QStringLiteral("minimum_quantity")).toInteger(1);
        if (itemId <= 0 || minimum <= 0
            || minimum > std::numeric_limits<std::uint32_t>::max()) {
            error = QStringLiteral("Item conditions require a positive item id and quantity.");
            return std::nullopt;
        }
        return ItemQuantityCondition{
            static_cast<std::uint64_t>(itemId), static_cast<std::uint32_t>(minimum)};
    }
    if (type == QLatin1String("gold_at_least")) {
        const auto amount = object.value(QStringLiteral("amount")).toInteger();
        if (amount < 0) {
            error = QStringLiteral("Gold requirements cannot be negative.");
            return std::nullopt;
        }
        return GoldAtLeastCondition{static_cast<std::uint64_t>(amount)};
    }
    if (type == QLatin1String("affinity_at_least")) {
        return AffinityAtLeastCondition{
            object.value(QStringLiteral("npc_id")).toString(),
            object.value(QStringLiteral("minimum")).toInt()};
    }
    if (type == QLatin1String("affinity_at_most")) {
        return AffinityAtMostCondition{
            object.value(QStringLiteral("npc_id")).toString(),
            object.value(QStringLiteral("maximum")).toInt()};
    }
    if (type == QLatin1String("character_attribute_is")) {
        return CharacterAttributeCondition{
            object.value(QStringLiteral("attribute")).toString(),
            object.value(QStringLiteral("value")).toString()};
    }
    if (type == QLatin1String("chapter_completed")) {
        return ChapterCompletedCondition{object.value(QStringLiteral("chapter_id")).toString()};
    }
    if (type == QLatin1String("npc_remembers") || type == QLatin1String("npc_missing_memory")) {
        return NpcMemoryCondition{
            object.value(QStringLiteral("npc_id")).toString(),
            object.value(QStringLiteral("key")).toString(),
            type == QLatin1String("npc_remembers")};
    }
    error = QStringLiteral("Unsupported condition type '%1'.").arg(type);
    return std::nullopt;
}

std::optional<StoryEffect> parseEffect(const QJsonObject& object, QString& error)
{
    const QString type = object.value(QStringLiteral("type")).toString();
    if (type == QLatin1String("set_flag") || type == QLatin1String("remove_flag")) {
        return SetFlagEffect{object.value(QStringLiteral("key")).toString(),
                             type == QLatin1String("set_flag")};
    }
    if (type == QLatin1String("change_affinity")) {
        return ChangeAffinityEffect{object.value(QStringLiteral("npc_id")).toString(),
                                    object.value(QStringLiteral("amount")).toInt()};
    }
    if (type == QLatin1String("change_gold")) {
        return ChangeGoldEffect{object.value(QStringLiteral("amount")).toInteger()};
    }
    if (type == QLatin1String("change_item")) {
        const auto itemId = object.value(QStringLiteral("item_id")).toInteger();
        const auto quantity = object.value(QStringLiteral("quantity")).toInteger();
        if (itemId <= 0 || quantity < std::numeric_limits<std::int32_t>::min()
            || quantity > std::numeric_limits<std::int32_t>::max()) {
            error = QStringLiteral("Item effect id or quantity is outside the supported range.");
            return std::nullopt;
        }
        return ChangeItemEffect{
            static_cast<std::uint64_t>(itemId), static_cast<std::int32_t>(quantity)};
    }
    if (type == QLatin1String("move_to_scene")) {
        return MoveToSceneEffect{object.value(QStringLiteral("scene_id")).toString()};
    }
    if (type == QLatin1String("complete_chapter")) return CompleteChapterEffect{};
    if (type == QLatin1String("unlock_chapter")) {
        return UnlockChapterEffect{object.value(QStringLiteral("chapter_id")).toString()};
    }
    if (type == QLatin1String("transition_to_chapter")) {
        return TransitionChapterEffect{object.value(QStringLiteral("chapter_id")).toString()};
    }
    if (type == QLatin1String("set_npc_memory")
        || type == QLatin1String("remove_npc_memory")) {
        return SetNpcMemoryEffect{
            object.value(QStringLiteral("npc_id")).toString(),
            object.value(QStringLiteral("key")).toString(),
            type == QLatin1String("set_npc_memory")};
    }
    error = QStringLiteral("Unsupported effect type '%1'.").arg(type);
    return std::nullopt;
}

} // namespace

StoryLoadResult StoryContentLoader::loadFile(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {std::nullopt, {QStringLiteral("Cannot open story content '%1'.").arg(path)}};
    }
    return loadJson(file.readAll(), path);
}

StoryLoadResult StoryContentLoader::loadJson(const QByteArray& json, const QString& sourceName) const
{
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return {std::nullopt,
                {QStringLiteral("%1: invalid JSON at offset %2: %3")
                     .arg(sourceName)
                     .arg(parseError.offset)
                     .arg(parseError.errorString())}};
    }

    const auto root = document.object();
    ChapterDefinition chapter;
    chapter.schemaVersion = root.value(QStringLiteral("schema_version")).toInt();
    chapter.chapterId = root.value(QStringLiteral("chapter_id")).toString();
    chapter.contentVersion = root.value(QStringLiteral("content_version")).toInt();
    chapter.entrySceneId = root.value(QStringLiteral("entry_scene")).toString();
    chapter.title = localizedText(root, "title");
    chapter.completionFlag = root.value(QStringLiteral("completion_flag")).toString();
    chapter.completionText = localizedText(root, "completion_text");
    chapter.allowActionsAfterCompletion =
        root.value(QStringLiteral("allow_actions_after_completion")).toBool(false);

    QStringList errors;
    const auto parseConditions = [&errors](const QJsonArray& values,
                                           QVector<StoryCondition>& destination,
                                           const QString& context) {
        for (qsizetype index = 0; index < values.size(); ++index) {
            QString error;
            auto condition = parseCondition(values.at(index).toObject(), error);
            if (!condition) {
                errors.push_back(QStringLiteral("%1 condition %2: %3")
                                     .arg(context)
                                     .arg(index)
                                     .arg(error));
                continue;
            }
            destination.push_back(std::move(*condition));
        }
    };
    const auto parseLines = [&parseConditions, &errors](
                                const QJsonArray& values,
                                QVector<StoryLineDefinition>& destination,
                                const QString& context) {
        for (qsizetype index = 0; index < values.size(); ++index) {
            const auto object = values.at(index).toObject();
            const QString type = object.value(QStringLiteral("type")).toString();
            StoryLineDefinition line;
            if (type == QLatin1String("narration")) {
                line.kind = StoryLineKind::Narration;
            } else if (type == QLatin1String("npc_dialogue")) {
                line.kind = StoryLineKind::NpcDialogue;
            } else if (type == QLatin1String("system_notice")) {
                line.kind = StoryLineKind::SystemNotice;
            } else {
                errors.push_back(QStringLiteral("%1 line %2: unsupported line type '%3'.")
                                     .arg(context).arg(index).arg(type));
                continue;
            }
            line.actorId = object.value(QStringLiteral("actor_id")).toString();
            line.speaker = localizedText(object, "speaker");
            line.text = localizedText(object, "text");
            parseConditions(object.value(QStringLiteral("conditions")).toArray(),
                            line.conditions,
                            QStringLiteral("%1 line %2").arg(context).arg(index));
            destination.push_back(std::move(line));
        }
    };

    parseConditions(root.value(QStringLiteral("requirements")).toArray(),
                    chapter.requirements, QStringLiteral("chapter"));
    parseConditions(root.value(QStringLiteral("completion_conditions")).toArray(),
                    chapter.completionConditions, QStringLiteral("chapter completion"));

    for (const auto& sceneValue : root.value(QStringLiteral("scenes")).toArray()) {
        const auto sceneObject = sceneValue.toObject();
        SceneDefinition scene;
        scene.id = sceneObject.value(QStringLiteral("id")).toString();
        scene.title = localizedText(sceneObject, "title");
        scene.narration = localizedText(sceneObject, "narration");
        scene.revisitNarration = localizedText(sceneObject, "revisit_narration");
        scene.actorIds = stringList(sceneObject.value(QStringLiteral("actors")));
        parseLines(sceneObject.value(QStringLiteral("arrival_lines")).toArray(),
                   scene.arrivalLines, QStringLiteral("scene '%1' arrival").arg(scene.id));

        for (const auto& actionValue : sceneObject.value(QStringLiteral("actions")).toArray()) {
            const auto actionObject = actionValue.toObject();
            StoryActionDefinition action;
            action.id = actionObject.value(QStringLiteral("id")).toString();
            action.label = localizedText(actionObject, "label");
            action.playerText = localizedText(actionObject, "player_text");
            action.aliases = stringList(actionObject.value(QStringLiteral("aliases")));
            parseConditions(actionObject.value(QStringLiteral("conditions")).toArray(),
                            action.conditions,
                            QStringLiteral("scene '%1', action '%2'").arg(scene.id, action.id));

            const auto effectValues = actionObject.value(QStringLiteral("effects")).toArray();
            for (qsizetype index = 0; index < effectValues.size(); ++index) {
                QString error;
                auto effect = parseEffect(effectValues.at(index).toObject(), error);
                if (!effect) {
                    errors.push_back(
                        QStringLiteral("scene '%1', action '%2' effect %3: %4")
                            .arg(scene.id, action.id)
                            .arg(index)
                            .arg(error));
                    continue;
                }
                action.effects.push_back(std::move(*effect));
            }
            parseLines(actionObject.value(QStringLiteral("response_lines")).toArray(),
                       action.responseLines,
                       QStringLiteral("scene '%1', action '%2' response")
                           .arg(scene.id, action.id));
            scene.actions.push_back(std::move(action));
        }
        chapter.scenes.push_back(std::move(scene));
    }

    if (!errors.isEmpty()) return {std::nullopt, errors};
    return {std::move(chapter), {}};
}

} // namespace arcane::application::story
