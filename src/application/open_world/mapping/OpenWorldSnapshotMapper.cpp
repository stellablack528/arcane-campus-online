#include "application/open_world/mapping/OpenWorldSnapshotMapper.hpp"

#include "application/story/StoryTypes.hpp"

#include <QByteArray>

namespace arcane::application::open_world {
namespace {

std::string utf8(const QString& value)
{
    const QByteArray encoded = value.toUtf8();
    return {encoded.constData(), static_cast<std::size_t>(encoded.size())};
}

std::set<std::string> strings(const QSet<QString>& values)
{
    std::set<std::string> result;
    for (const auto& value : values) result.insert(utf8(value));
    return result;
}

} // namespace

OpenWorldSnapshotMappingResult OpenWorldSnapshotMapper::map(
    const story::StoryRuntimeState& state)
{
    if (state.characterId == 0) {
        return {std::nullopt, "Cannot create an open-world snapshot without a character id."};
    }
    if (state.chapterId.trimmed().isEmpty()) {
        return {std::nullopt, "Cannot create an open-world snapshot without a chapter id."};
    }
    if (state.currentSceneId.trimmed().isEmpty()) {
        return {std::nullopt, "Cannot create an open-world snapshot without a current scene."};
    }
    if (state.contentVersion <= 0) {
        return {std::nullopt, "Cannot create an open-world snapshot from invalid story content."};
    }

    const bool mainStoryComplete = state.flags.contains(
        QStringLiteral("main_story_complete"));
    const bool handoffReady = state.flags.contains(
        QStringLiteral("open_world_handoff_ready"));
    if (handoffReady && !mainStoryComplete) {
        return {std::nullopt,
                "The open-world handoff flag requires the completed main-story flag."};
    }

    OpenWorldSnapshot snapshot;
    snapshot.characterId = state.characterId;
    snapshot.chapterId = utf8(state.chapterId);
    snapshot.contentVersion = state.contentVersion;
    // StoryRuntimeService increments revision once for every accepted action.
    // The boundary names that meaning explicitly instead of exposing a storage term.
    snapshot.acceptedStoryTurns = state.revision;
    snapshot.currentSceneId = utf8(state.currentSceneId);
    snapshot.gold = state.gold;
    snapshot.flags = strings(state.flags);
    snapshot.visitedScenes = strings(state.visitedScenes);
    snapshot.completedChapters = strings(state.completedChapters);
    snapshot.unlockedChapters = strings(state.unlockedChapters);
    snapshot.chapterCompleted = state.chapterCompleted;
    snapshot.mainStoryComplete = mainStoryComplete;
    snapshot.openWorldHandoffReady = handoffReady;

    for (auto it = state.inventory.cbegin(); it != state.inventory.cend(); ++it)
        snapshot.inventory.emplace(it.key(), it.value());
    for (auto it = state.npcAffinities.cbegin(); it != state.npcAffinities.cend(); ++it)
        snapshot.npcRelationships.emplace(utf8(it.key()), it.value());
    for (auto it = state.npcMemoryFacts.cbegin(); it != state.npcMemoryFacts.cend(); ++it)
        snapshot.npcMemoryFacts.emplace(utf8(it.key()), strings(it.value()));
    for (auto it = state.characterAttributes.cbegin();
         it != state.characterAttributes.cend(); ++it) {
        snapshot.characterAttributes.emplace(utf8(it.key()), utf8(it.value()));
    }

    const auto house = state.characterAttributes.constFind(QStringLiteral("house"));
    if (house != state.characterAttributes.cend()) snapshot.house = utf8(house.value());

    return {std::move(snapshot), {}};
}

} // namespace arcane::application::open_world
