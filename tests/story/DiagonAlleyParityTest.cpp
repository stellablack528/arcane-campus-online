#include "application/diagon_alley/DiagonAlleyCatalog.hpp"
#include "application/diagon_alley/DiagonAlleyService.hpp"
#include "application/story/StoryContentValidator.hpp"
#include "application/story/StoryEngine.hpp"
#include "application/story/adapters/DiagonAlleyChapterAdapter.hpp"
#include "ui/I18n.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSet>
#include <QStringConverter>
#include <QTextStream>

#include <cstdint>
#include <iostream>

namespace alley = arcane::application::diagon_alley;
namespace story = arcane::application::story;

namespace {

struct Report {
    bool passed = true;
    QStringList lines;

    void check(bool condition, const QString& name, const QString& detail = {})
    {
        passed = passed && condition;
        lines.push_back(QStringLiteral("[%1] %2%3")
                            .arg(condition ? QStringLiteral("PASS") : QStringLiteral("FAIL"),
                                 name,
                                 detail.isEmpty() ? QString() : QStringLiteral(" | ") + detail));
    }
};

QString sortedList(const QSet<QString>& values)
{
    QStringList list(values.begin(), values.end());
    list.sort();
    return list.join(QStringLiteral(", "));
}

QString inventoryText(const QHash<std::uint64_t, std::uint32_t>& inventory)
{
    QStringList result;
    for (auto it = inventory.cbegin(); it != inventory.cend(); ++it)
        result.push_back(QStringLiteral("%1x%2").arg(it.key()).arg(it.value()));
    result.sort();
    return result.join(QStringLiteral(", "));
}

QString affinityText(const QHash<QString, int>& affinities)
{
    QStringList result;
    for (auto it = affinities.cbegin(); it != affinities.cend(); ++it)
        result.push_back(QStringLiteral("%1=%2").arg(it.key()).arg(it.value()));
    result.sort();
    return result.join(QStringLiteral(", "));
}

QString memoryText(const QHash<QString, QSet<QString>>& memory)
{
    QStringList result;
    for (auto it = memory.cbegin(); it != memory.cend(); ++it)
        result.push_back(QStringLiteral("%1=[%2]").arg(it.key(), sortedList(it.value())));
    result.sort();
    return result.join(QStringLiteral(", "));
}

bool writeReport(const QString& path, const Report& report)
{
    if (path.isEmpty()) return true;
    const QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath())) return false;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;
    QTextStream output(&file);
    output.setEncoding(QStringConverter::Utf8);
    output << "Arcane Campus Online - Diagon Alley migration equivalence report\n";
    output << "===============================================================\n";
    for (const auto& line : report.lines) output << line << '\n';
    output << "---------------------------------------------------------------\n";
    output << "OVERALL: " << (report.passed ? "PASS" : "FAIL") << '\n';
    output << "Legacy production entry remains active; generic runtime is migration-gated.\n";
    return true;
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    arcane::ui::I18n::instance().setLanguage(arcane::ui::I18n::Lang::Chinese);
    const QStringList arguments = app.arguments();
    const qsizetype reportIndex = arguments.indexOf(QStringLiteral("--report"));
    const QString reportPath = reportIndex >= 0 && reportIndex + 1 < arguments.size()
        ? arguments.at(reportIndex + 1) : QString();

    Report report;

    alley::DiagonAlleyCatalog catalog;
    const bool catalogLoaded = catalog.load();
    report.check(catalogLoaded, QStringLiteral("Legacy Diagon Alley content package loads"));
    if (!catalogLoaded) {
        const bool written = writeReport(reportPath, report);
        return report.passed && written ? 0 : 1;
    }

    alley::DiagonAlleyService legacy;
    std::uint64_t legacyGold = 50;
    QHash<std::uint64_t, std::uint32_t> legacyInventory;
    QHash<QString, int> legacyAffinities;
    QHash<QString, QSet<QString>> legacyMemory;
    QObject::connect(&legacy, &alley::DiagonAlleyService::purchaseRequested,
                     [&](const QString& purchaseId, std::uint64_t itemId, std::uint64_t price) {
                         if (legacyGold < price) {
                             legacy.resolvePurchase(purchaseId, false, legacyGold,
                                                    QStringLiteral("Insufficient gold"));
                             return;
                         }
                         legacyGold -= price;
                         legacyInventory[itemId] = legacyInventory.value(itemId, 0) + 1;
                         legacy.resolvePurchase(purchaseId, true, legacyGold, {});
                     });
    QObject::connect(&legacy, &alley::DiagonAlleyService::npcMemoryChanged,
                     [&](const QString& npcId, int affinity, const QStringList& flags) {
                         legacyAffinities[npcId] = affinity;
                         legacyMemory[npcId] = QSet<QString>(flags.begin(), flags.end());
                     });

    legacy.start(42, QStringLiteral("street"), {QStringLiteral("wand")});
    const QStringList legacyRoute{
        QStringLiteral("去丽痕书店"), QStringLiteral("和店主交谈"),
        QStringLiteral("购买课本"), QStringLiteral("回街上"),
        QStringLiteral("去摩金夫人长袍店"), QStringLiteral("不理"),
        QStringLiteral("买校服"), QStringLiteral("回街上"),
        QStringLiteral("去药材店"), QStringLiteral("和店主交谈"),
        QStringLiteral("和店主交谈"), QStringLiteral("购买坩埚"),
        QStringLiteral("回街上"), QStringLiteral("去神奇动物商店"),
        QStringLiteral("不理"), QStringLiteral("不理"),
        QStringLiteral("选择猫头鹰"),
        QStringLiteral("回街上"), QStringLiteral("去丽痕书店"),
        QStringLiteral("和店主交谈"),
    };
    bool legacyAccepted = true;
    for (const auto& command : legacyRoute) legacyAccepted &= legacy.submitText(command);
    report.check(legacyAccepted, QStringLiteral("Legacy runtime accepts the complete route"));

    story::adapters::DiagonAlleyChapterAdapter adapter;
    const story::ChapterDefinition chapter = adapter.build(catalog);
    story::StoryContentValidator validator;
    const auto validation = validator.validate(chapter);
    report.check(validation.isValid(), QStringLiteral("Adapted chapter passes generic validation"),
                 validation.errors.join(QStringLiteral("; ")));

    story::StoryRuntimeState generic;
    generic.characterId = 42;
    generic.chapterId = chapter.chapterId;
    generic.currentSceneId = chapter.entrySceneId;
    generic.gold = 50;
    generic.flags = {QStringLiteral("wand"), QStringLiteral("visited_street")};

    const QStringList genericRoute{
        QStringLiteral("enter_flourish_and_blotts"),
        QStringLiteral("talk_bookseller_merrythought_first"),
        QStringLiteral("buy_first_year_books"),
        QStringLiteral("return_flourish_and_blotts_to_street"),
        QStringLiteral("enter_madam_malkins"),
        QStringLiteral("ignore_madam_malkin_first"),
        QStringLiteral("buy_school_robes"),
        QStringLiteral("return_madam_malkins_to_street"),
        QStringLiteral("enter_apothecary"),
        QStringLiteral("talk_apothecary_clerk_ives_first"),
        QStringLiteral("talk_apothecary_clerk_ives_repeat"),
        QStringLiteral("buy_potion_kit"),
        QStringLiteral("return_apothecary_to_street"),
        QStringLiteral("enter_magical_menagerie"),
        QStringLiteral("ignore_menagerie_keeper_wren_first"),
        QStringLiteral("ignore_menagerie_keeper_wren_repeat"),
        QStringLiteral("buy_adopt_owl"),
        QStringLiteral("return_magical_menagerie_to_street"),
        QStringLiteral("enter_flourish_and_blotts"),
        QStringLiteral("talk_bookseller_merrythought_repeat"),
    };
    story::StoryEngine engine;
    bool genericAccepted = validation.isValid() && engine.canStart(chapter, generic);
    QString genericError;
    for (const auto& actionId : genericRoute) {
        if (!genericAccepted) break;
        const auto result = engine.execute(chapter, generic, actionId);
        if (!result.success) {
            genericAccepted = false;
            genericError = actionId + QStringLiteral(": ") + result.message;
            break;
        }
        if (!engine.applyDelta(generic, result.delta, &genericError)) {
            genericAccepted = false;
            genericError = actionId + QStringLiteral(": ") + genericError;
            break;
        }
    }
    report.check(genericAccepted, QStringLiteral("Generic runtime accepts the complete route"),
                 genericError);

    const QStringList legacyFlagList = legacy.flags();
    const QSet<QString> legacyFlags(legacyFlagList.begin(), legacyFlagList.end());
    report.check(legacyGold == generic.gold, QStringLiteral("Gold balance is equivalent"),
                 QStringLiteral("legacy=%1, generic=%2").arg(legacyGold).arg(generic.gold));
    report.check(legacyInventory == generic.inventory,
                 QStringLiteral("Purchased inventory is equivalent"),
                 QStringLiteral("legacy={%1}, generic={%2}")
                     .arg(inventoryText(legacyInventory), inventoryText(generic.inventory)));
    report.check(legacyFlags == generic.flags, QStringLiteral("Progress flags are equivalent"),
                 QStringLiteral("legacy={%1}, generic={%2}")
                     .arg(sortedList(legacyFlags), sortedList(generic.flags)));
    report.check(legacyAffinities == generic.npcAffinities,
                 QStringLiteral("NPC affinities are equivalent"),
                 QStringLiteral("legacy={%1}, generic={%2}")
                     .arg(affinityText(legacyAffinities), affinityText(generic.npcAffinities)));
    report.check(legacyMemory == generic.npcMemoryFacts,
                 QStringLiteral("NPC memory facts are equivalent"),
                 QStringLiteral("legacy={%1}, generic={%2}")
                     .arg(memoryText(legacyMemory), memoryText(generic.npcMemoryFacts)));
    report.check(legacy.currentSceneId() == generic.currentSceneId,
                 QStringLiteral("Post-completion exploration and final scene are equivalent"),
                 QStringLiteral("legacy=%1, generic=%2")
                     .arg(legacy.currentSceneId(), generic.currentSceneId));
    report.check(legacyFlags.contains(QStringLiteral("shopping_complete"))
                     && generic.chapterCompleted
                     && generic.completedChapters.contains(chapter.chapterId),
                 QStringLiteral("Chapter completion is equivalent"));

    const bool reportWritten = writeReport(reportPath, report);
    if (!reportPath.isEmpty()) {
        std::cout << "Parity report: " << reportPath.toStdString() << '\n';
        if (!reportWritten) std::cerr << "FAILED: could not write parity report\n";
    }
    for (const auto& line : report.lines) std::cout << line.toStdString() << '\n';
    return report.passed && reportWritten ? 0 : 1;
}
