#include "application/story/StoryContentLoader.hpp"
#include "application/story/StoryRuntimeService.hpp"

#include <QCoreApplication>

#include <iostream>

namespace story = arcane::application::story;

namespace {

bool expect(bool condition, const char* message)
{
    if (condition) return true;
    std::cerr << "FAILED: " << message << '\n';
    return false;
}

const QByteArray kRuntimeChapter = R"json(
{
  "schema_version": 2,
  "chapter_id": "runtime_bridge_test",
  "content_version": 1,
  "entry_scene": "owlery",
  "title": {"zh": "运行桥测试", "en": "Runtime Bridge Test"},
  "completion_text": {"zh": "信中的墨迹安静下来。", "en": "The ink in the letter grows still."},
  "scenes": [
    {
      "id": "owlery",
      "title": {"zh": "猫头鹰棚屋", "en": "Owlery"},
      "narration": {"zh": "一封信落在你面前。", "en": "A letter lands before you."},
      "actors": ["owl_courier"],
      "arrival_lines": [
        {
          "type": "npc_dialogue",
          "actor_id": "owl_courier",
          "speaker": {"zh": "灰羽猫头鹰", "en": "Grey-feathered Owl"},
          "text": {"zh": "它用喙点了点信封。", "en": "It taps the envelope with its beak."}
        }
      ],
      "actions": [
        {
          "id": "open_letter",
          "label": {"zh": "打开信封", "en": "Open the letter"},
          "player_text": {"zh": "你拆开了信封。", "en": "You open the envelope."},
          "aliases": ["打开信封", "拆信", "open letter"],
          "effects": [
            {"type": "set_flag", "key": "letter_opened"},
            {"type": "complete_chapter"}
          ],
          "response_lines": [
            {
              "type": "system_notice",
              "text": {"zh": "信件已阅读。", "en": "Letter read."}
            }
          ]
        }
      ]
    }
  ]
}
)json";

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    bool passed = true;

    story::StoryContentLoader loader;
    const auto loaded = loader.loadJson(kRuntimeChapter, QStringLiteral("runtime-service-test"));
    passed &= expect(loaded.success() && loaded.chapter.has_value(),
                     "runtime chapter loads");
    if (!loaded.chapter) return 1;

    story::StoryRuntimeService service;
    story::StoryRuntimeState initial;
    initial.characterId = 42;

    bool commitReached = false;
    bool outputBeforeCommit = false;
    int headerCount = 0;
    int narrativeCount = 0;
    int choicesCount = 0;
    int stateCount = 0;
    int errorCount = 0;
    QStringList speakers;
    QStringList latestChoices;

    const auto markOutput = [&] {
        if (!commitReached) outputBeforeCommit = true;
    };
    QObject::connect(&service, &story::StoryRuntimeService::sceneHeaderChanged,
                     [&](const QString&) { markOutput(); ++headerCount; });
    QObject::connect(&service, &story::StoryRuntimeService::narrativeProduced,
                     [&](const QString& speaker, const QString&) {
                         markOutput();
                         speakers.push_back(speaker);
                         ++narrativeCount;
                     });
    QObject::connect(&service, &story::StoryRuntimeService::choicesPresented,
                     [&](const QStringList& labels) {
                         markOutput();
                         latestChoices = labels;
                         ++choicesCount;
                     });
    QObject::connect(&service, &story::StoryRuntimeService::runtimeStateChanged,
                     [&](const QString&, const QStringList&) { markOutput(); ++stateCount; });
    QObject::connect(&service, &story::StoryRuntimeService::inputHintChanged,
                     [&](const QString&) { markOutput(); });
    QObject::connect(&service, &story::StoryRuntimeService::runtimeError,
                     [&](const QString&) { ++errorCount; });

    service.setCommitter([&](const story::ChapterDefinition&,
                             const story::StoryRuntimeState&,
                             const story::StoryExecutionResult&,
                             QString&) {
        commitReached = true;
        return true;
    });
    passed &= expect(service.start(*loaded.chapter, initial),
                     "runtime starts through the commit boundary");
    passed &= expect(service.isActive() && !outputBeforeCommit,
                     "scene output is emitted only after commit succeeds");
    passed &= expect(headerCount == 1 && narrativeCount == 2
                         && choicesCount == 1 && stateCount == 1
                         && latestChoices == QStringList{QStringLiteral("打开信封")},
                     "scene events map onto the existing central narrative signals");
    passed &= expect(service.state()
                         && service.state()->visitedScenes.contains(QStringLiteral("owlery"))
                         && service.state()->revision == 1
                         && service.visibleActionIds()
                                == QStringList{QStringLiteral("open_letter")},
                     "committed scene state and choice ids are retained");

    const int narrativeBeforeFailure = narrativeCount;
    const int choicesBeforeFailure = choicesCount;
    const int stateBeforeFailure = stateCount;
    commitReached = false;
    service.setCommitter([](const story::ChapterDefinition&,
                            const story::StoryRuntimeState&,
                            const story::StoryExecutionResult&,
                            QString& error) {
        error = QStringLiteral("simulated persistence failure");
        return false;
    });
    passed &= expect(!service.submitChoice(0), "failed persistence rejects the action");
    passed &= expect(narrativeCount == narrativeBeforeFailure
                         && choicesCount == choicesBeforeFailure
                         && stateCount == stateBeforeFailure
                         && service.state()
                         && service.state()->revision == 1
                         && !service.state()->flags.contains(QStringLiteral("letter_opened"))
                         && errorCount == 1,
                     "failed persistence emits no story output and leaves state untouched");

    commitReached = false;
    service.setCommitter([&](const story::ChapterDefinition&,
                             const story::StoryRuntimeState&,
                             const story::StoryExecutionResult&,
                             QString&) {
        commitReached = true;
        return true;
    });
    passed &= expect(service.submitText(QStringLiteral("请帮我打开信封")),
                     "free-text alias resolves to the available action");
    passed &= expect(!outputBeforeCommit && service.state()
                         && service.state()->revision == 2
                         && service.state()->chapterCompleted
                         && service.state()->flags.contains(QStringLiteral("letter_opened")),
                     "successful action commits before publishing its events");
    passed &= expect(speakers.contains(QStringLiteral("你"))
                         && speakers.contains(QStringLiteral("系统"))
                         && latestChoices.isEmpty(),
                     "player, system, completion, and cleared-choice events reach the UI bridge");

    return passed ? 0 : 1;
}
