#include "application/prologue/WandLoreService.hpp"

#include "core/DeepSeekClient.hpp"
#include "ui/I18n.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace arcane::application::prologue {

namespace {

struct WandPart {
    QString englishName;
    QString chineseName;
    QString traits;
};

WandPart findPart(const char* collectionName, const QString& id)
{
    QFile file(QStringLiteral(":/data/wand_customization.json"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {id, id, {}};
    }

    const auto root = QJsonDocument::fromJson(file.readAll()).object();
    const auto entries = root.value(QLatin1String(collectionName)).toArray();
    for (const auto& entry : entries) {
        const auto object = entry.toObject();
        if (object.value(QStringLiteral("id")).toString() == id) {
            return {
                object.value(QStringLiteral("name_en")).toString(),
                object.value(QStringLiteral("name_zh")).toString(),
                object.value(QStringLiteral("traits")).toString(),
            };
        }
    }
    return {id, id, {}};
}

QString languageInstruction()
{
    return arcane::ui::I18n::instance().language() == arcane::ui::I18n::Lang::Chinese
        ? QStringLiteral("请使用自然、克制、具有魔法世界氛围的简体中文。")
        : QStringLiteral("Write in natural, restrained English with a magical-world atmosphere.");
}

} // namespace

WandLoreService::WandLoreService(QObject* parent)
    : QObject(parent)
    , client_(std::make_unique<core::DeepSeekClient>(this))
{
    connect(client_.get(), &core::DeepSeekClient::replyReceived,
            this, [this](const QString&, const QString&, const QString& text) {
                if (!requestPending_) {
                    return;
                }
                requestPending_ = false;
                emit readingReady(text.trimmed(), true);
            });
    connect(client_.get(), &core::DeepSeekClient::errorOccurred,
            this, [this](const QString&) {
                if (!requestPending_) {
                    return;
                }
                requestPending_ = false;
                emit readingReady(localReading(pendingWand_), false);
            });
}

WandLoreService::~WandLoreService() = default;

void WandLoreService::requestReading(const WandConfig& wand)
{
    pendingWand_ = wand;

    if (!core::DeepSeekClient::hasApiKey()) {
        emit readingReady(localReading(wand), false);
        return;
    }

    const auto wood = findPart("official_wood_options", wand.woodId);
    const auto core = findPart("official_ollivander_core_options", wand.coreId);
    const QString systemPrompt = QStringLiteral(
        "You are Mr. Ollivander giving a first-year witch or wizard a brief wand reading. "
        "Base the interpretation only on the supplied wand wood, core, length and flexibility. "
        "Write 2-3 original sentences. Describe tendencies and affinities rather than declaring a fixed destiny. "
        "Do not quote or imitate text from the novels, do not mention an AI, and do not add headings. %1")
        .arg(languageInstruction());
    const QString userPrompt = QStringLiteral(
        "Wood: %1 (%2). Canon-inspired archive traits: %3. "
        "Core: %4 (%5). Core traits: %6. Length: %7 inches. Flexibility: %8.")
        .arg(wood.englishName,
             wood.chineseName,
             wood.traits,
             core.englishName,
             core.chineseName,
             core.traits,
             QString::number(wand.lengthInches, 'f', 2),
             wand.flexibility);

    requestPending_ = true;
    client_->sendChat(systemPrompt,
                      userPrompt,
                      QStringLiteral("wand-reading"),
                      QStringLiteral("Mr. Ollivander"));
}

QString WandLoreService::localReading(const WandConfig& wand) const
{
    const auto wood = findPart("official_wood_options", wand.woodId);
    const auto core = findPart("official_ollivander_core_options", wand.coreId);
    const bool chinese = arcane::ui::I18n::instance().language() == arcane::ui::I18n::Lang::Chinese;

    const QString adaptability = wand.flexibility.contains(QStringLiteral("rigid"), Qt::CaseInsensitive)
            || wand.flexibility.contains(QStringLiteral("unyield"), Qt::CaseInsensitive)
        ? (chinese ? QStringLiteral("它偏爱意志坚定、不会轻易偏离原则的主人。")
                   : QStringLiteral("It favours an owner whose convictions do not bend easily."))
        : (chinese ? QStringLiteral("它会回应愿意学习、也能随经历不断成长的主人。")
                   : QStringLiteral("It responds to an owner willing to learn and grow through experience."));

    if (chinese) {
        return QStringLiteral(
            "%1与%2构成了一种很有主见的搭配：它重视的不是表面的声势，而是你在关键时刻如何选择。%3"
            "%4 英寸的长度让它更贴近你自己的施法节奏——这不是命运的判词，只是它此刻对你的回答。")
            .arg(wood.chineseName,
                 core.chineseName,
                 adaptability,
                 QString::number(wand.lengthInches, 'f', 2));
    }

    return QStringLiteral(
        "%1 and %2 make a discerning pairing: it cares less for display than for the choices you make when they matter. "
        "%3 At %4 inches, it is tuned to your own rhythm of spellwork—not a verdict on your destiny, only its answer to you today.")
        .arg(wood.englishName,
             core.englishName,
             adaptability,
             QString::number(wand.lengthInches, 'f', 2));
}

} // namespace arcane::application::prologue
