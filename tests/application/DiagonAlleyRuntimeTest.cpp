#include "application/diagon_alley/DiagonAlleyActionParser.hpp"
#include "application/diagon_alley/DiagonAlleyCatalog.hpp"
#include "application/diagon_alley/DiagonAlleyService.hpp"
#include "ui/I18n.hpp"

#include <QCoreApplication>
#include <QHash>
#include <QStringList>

#include <cstdint>
#include <algorithm>
#include <iostream>

namespace alley = arcane::application::diagon_alley;

namespace {

bool expect(bool condition, const char* message)
{
    if (condition) return true;
    std::cerr << "FAILED: " << message << '\n';
    return false;
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    arcane::ui::I18n::instance().setLanguage(arcane::ui::I18n::Lang::Chinese);
    bool passed = true;

    alley::DiagonAlleyCatalog catalog;
    passed &= expect(catalog.load(), "content package loads from Qt resources");
    passed &= expect(catalog.shops().size() == 4, "content package exposes four shops");

    alley::DiagonAlleyActionParser parser;
    const auto enter = parser.parse(QStringLiteral("我想去丽痕书店看看"),
                                    QStringLiteral("street"), catalog);
    passed &= expect(enter.kind == alley::ActionKind::EnterShop
                         && enter.targetId == QLatin1String("flourish_and_blotts"),
                     "Chinese free-text input resolves a shop");

    const auto buy = parser.parse(QStringLiteral("购买课本"),
                                  QStringLiteral("flourish_and_blotts"), catalog);
    passed &= expect(buy.kind == alley::ActionKind::Buy
                         && buy.targetId == QLatin1String("first_year_books"),
                     "Chinese free-text input resolves a purchase");

    alley::DiagonAlleyService service;
    QStringList latestChoices;
    QStringList narrativeBodies;
    int savedAffinity = 0;
    QStringList savedMemoryFlags;
    std::uint64_t remainingGold = 50;

    QObject::connect(&service, &alley::DiagonAlleyService::choicesPresented,
                     [&latestChoices](const QStringList& choices) { latestChoices = choices; });
    QObject::connect(&service, &alley::DiagonAlleyService::narrativeProduced,
                     [&narrativeBodies](const QString&, const QString& body) {
                         narrativeBodies.push_back(body);
                     });
    QObject::connect(&service, &alley::DiagonAlleyService::npcMemoryChanged,
                     [&savedAffinity, &savedMemoryFlags](const QString&, int affinity,
                                                        const QStringList& flags) {
                         savedAffinity = affinity;
                         savedMemoryFlags = flags;
                     });
    QObject::connect(&service, &alley::DiagonAlleyService::purchaseRequested,
                     [&service, &remainingGold](const QString& id, std::uint64_t,
                                                std::uint64_t price) {
                         remainingGold -= price;
                         service.resolvePurchase(id, true, remainingGold, {});
                     });

    const auto submit = [&service, &passed](const QString& text) {
        passed &= expect(service.submitText(text), "active story runtime accepts typed input");
    };

    service.start(42, QStringLiteral("street"), {QStringLiteral("wand")});
    passed &= expect(service.isActive(), "story runtime starts");
    passed &= expect(latestChoices.size() == 5, "street offers four shops and a shopping list");
    passed &= expect(!narrativeBodies.isEmpty()
                         && narrativeBodies.first().contains(QStringLiteral("按钮只是建议")),
                     "first street visit uses the opening narration");

    submit(QStringLiteral("去丽痕书店"));
    submit(QStringLiteral("和店主交谈"));
    passed &= expect(savedAffinity == 1 && savedMemoryFlags.contains(QStringLiteral("talked")),
                     "NPC conversation records affinity and memory");

    alley::DiagonAlleyService resumedService;
    QStringList resumedNarratives;
    QObject::connect(&resumedService, &alley::DiagonAlleyService::narrativeProduced,
                     [&resumedNarratives](const QString&, const QString& body) {
                         resumedNarratives.push_back(body);
                     });
    resumedService.start(
        42, QStringLiteral("flourish_and_blotts"), {QStringLiteral("wand")},
        {{QStringLiteral("bookseller_merrythought"), 1}},
        {{QStringLiteral("bookseller_merrythought"), {QStringLiteral("talked")}}});
    const bool rememberedGreeting = std::any_of(
        resumedNarratives.cbegin(), resumedNarratives.cend(), [](const QString& body) {
            return body.contains(QStringLiteral("又见面了"));
        });
    passed &= expect(rememberedGreeting, "restored NPC memory changes the next greeting");

    submit(QStringLiteral("购买课本"));
    submit(QStringLiteral("回街上"));
    submit(QStringLiteral("去摩金夫人长袍店"));
    submit(QStringLiteral("买校服"));
    submit(QStringLiteral("回街上"));
    submit(QStringLiteral("去药材店"));
    submit(QStringLiteral("购买坩埚"));
    submit(QStringLiteral("回街上"));
    submit(QStringLiteral("去神奇动物商店"));
    submit(QStringLiteral("选择猫头鹰"));

    const auto flags = service.flags();
    passed &= expect(flags.contains(QStringLiteral("books"))
                         && flags.contains(QStringLiteral("robes"))
                         && flags.contains(QStringLiteral("potions"))
                         && flags.contains(QStringLiteral("pet")),
                     "all four shopping branches can be completed");
    passed &= expect(flags.contains(QStringLiteral("shopping_complete")),
                     "shopping completion unlocks after every required branch");
    passed &= expect(remainingGold == 10, "purchase prices are applied consistently");

    const auto previousNarrativeCount = narrativeBodies.size();
    submit(QStringLiteral("购买另一只猫"));
    passed &= expect(narrativeBodies.size() > previousNarrativeCount,
                     "a completed purchase receives an in-world response");

    return passed ? 0 : 1;
}
