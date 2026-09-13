#include "application/prologue/PrologueService.hpp"

#include <QString>

namespace arcane::application::prologue {

namespace {

// 选项标签 → 数据 id 的映射表。
struct Option {
    const char* label;
    const char* id;
};

// 阶段1：宠物
constexpr Option kPets[] = {
    {"Owl",  "owl"},
    {"Cat",  "cat"},
    {"Toad", "toad"},
};

// 阶段2a：魔杖木材（精选 6 种，对应 wand_customization.json 的 id）
constexpr Option kWoods[] = {
    {"Holly",   "holly"},
    {"Yew",     "yew"},
    {"Oak",     "english_oak"},
    {"Elder",   "elder"},
    {"Vine",    "vine"},
    {"Willow",  "willow"},
};

// 阶段2b：杖芯
constexpr Option kCores[] = {
    {"Phoenix Feather",    "phoenix_feather"},
    {"Dragon Heartstring", "dragon_heartstring"},
    {"Unicorn Hair",       "unicorn_hair"},
};

// 阶段2c：长度（英寸）
constexpr double kLengths[] = {9.0, 10.0, 11.0, 12.0, 13.0, 14.0};

// 阶段2d：柔韧度
constexpr Option kFlexes[] = {
    {"Unyielding", "Unyielding"},
    {"Rigid",      "Rigid"},
    {"Supple",     "Supple"},
    {"Pliant",     "Pliant"},
};

// 富文本叙事包装：统一羊皮纸风格的段落。
QString narrative(const QString& speaker, const QString& body)
{
    return QStringLiteral(
        "<div style='margin:6px 0; padding:6px 0; border-bottom:1px dotted #3a3528;'>"
        "<span style='color:#e8d5a4; font-weight:700;'>%1</span><br>"
        "<span style='color:#d8d0bc;'>%2</span></div>")
        .arg(speaker, body);
}

QStringList labelsOf(const Option* opts, int count)
{
    QStringList out;
    for (int i = 0; i < count; ++i) out << QString::fromLatin1(opts[i].label);
    return out;
}

} // namespace

PrologueService::PrologueService(QObject* parent)
    : QObject(parent) {}

void PrologueService::begin()
{
    stage_ = PrologueStage::DiagonAlleyPet;
    emit stageChanged(stage_);
    enterPetStage();
}

void PrologueService::setScene(const QString& title, const QString& hint)
{
    emit sceneHeaderChanged(title);
    emit inputHintChanged(hint);
}

void PrologueService::emitNarrative(const QString& html)
{
    emit narrativeProduced(html);
}

void PrologueService::emitChoices(const QStringList& labels)
{
    emit choicesPresented(labels);
}

// ---------- 阶段1：对角巷·宠物 ----------

void PrologueService::enterPetStage()
{
    setScene(QStringLiteral("Diagon Alley \xe2\x80\x94 Magical Menagerie"),
             QStringLiteral("Choose your companion..."));
    emitNarrative(narrative(
        QStringLiteral("Narrator"),
        QStringLiteral(
            "The cobblestones of Diagon Alley thrum with wizarding life \xe2\x80\x94 "
            "owls hoot from the post office, cauldrons clatter, and the sweet smell "
            "of Sugar Quills drifts from Honeydukes.<br><br>"
            "You step into the Magical Menagerie. cages line the walls: a purple toad "
            "croaks lazily, a tabby cat watches you with knowing eyes, and a snowy owl "
            "ruffles its feathers and gives a soft hoot.<br><br>"
            "<i>Which companion calls to you?</i>")));
    emitChoices(labelsOf(kPets, static_cast<int>(sizeof(kPets) / sizeof(kPets[0]))));
}

// ---------- 阶段2：奥利凡德魔杖店 ----------

void PrologueService::enterWandStage()
{
    stage_ = PrologueStage::DiagonAlleyWand;
    wandStep_ = WandStep::Wood;
    emit stageChanged(stage_);
    setScene(QStringLiteral("Ollivanders \xe2\x80\x94 Makers of Fine Wands"),
             QStringLiteral("Let the wand choose you..."));

    emitNarrative(narrative(
        QStringLiteral("Mr. Ollivander"),
        QStringLiteral(
            "A bell tinkles as you enter the narrow, shabby shop. The walls are lined "
            "floor to ceiling with dusty wand boxes. An old wizard with pale eyes "
            "surveys you from behind the counter.<br><br>"
            "\xe2\x80\x9cCurious\xe2\x80\xa6 very curious,\xe2\x80\x9d he murmurs. "
            "\xe2\x80\x9cI remember every wand I've ever sold. The wand chooses the "
            "wizard, you know. Let us see which wood is meant for you.\xe2\x80\x9d")));
    emitChoices(labelsOf(kWoods, static_cast<int>(sizeof(kWoods) / sizeof(kWoods[0]))));
}

void PrologueService::advanceWandStep()
{
    switch (wandStep_) {
    case WandStep::Wood:
        // 已选木材，进入杖芯。
        wandStep_ = WandStep::Core;
        setScene(QStringLiteral("Ollivanders \xe2\x80\x94 Core"),
                 QStringLiteral("Choose the wand core..."));
        emitNarrative(narrative(
            QStringLiteral("Mr. Ollivander"),
            QStringLiteral(
                "\xe2\x80\x9cAh, an excellent wood. Now \xe2\x80\x94 the heart of the "
                "wand. The core gives the wand its temperament. Each is unique.\xe2\x80\x9d")));
        emitChoices(labelsOf(kCores, static_cast<int>(sizeof(kCores) / sizeof(kCores[0]))));
        break;

    case WandStep::Core:
        wandStep_ = WandStep::Length;
        setScene(QStringLiteral("Ollivanders \xe2\x80\x94 Length"),
                 QStringLiteral("Choose the wand length..."));
        emitNarrative(narrative(
            QStringLiteral("Mr. Ollivander"),
            QStringLiteral(
                "\xe2\x80\x9cA fine pairing. Now the length \xe2\x80\x94 it must suit "
                "your style of spellwork. Shorter wands favour precision; longer ones, "
                "grandeur.\xe2\x80\x9d")));
        {
            QStringList lens;
            for (double l : kLengths) lens << QString::number(l, 'f', 1) + QStringLiteral("\"");
            emitChoices(lens);
        }
        break;

    case WandStep::Length:
        wandStep_ = WandStep::Flex;
        setScene(QStringLiteral("Ollivanders \xe2\x80\x94 Flexibility"),
                 QStringLiteral("Choose the flexibility..."));
        emitNarrative(narrative(
            QStringLiteral("Mr. Ollivander"),
            QStringLiteral(
                "\xe2\x80\x9cAnd finally, the flexibility. Rigid wands favour those "
                "of stubborn will; supple ones grow with their master.\xe2\x80\x9d")));
        emitChoices(labelsOf(kFlexes, static_cast<int>(sizeof(kFlexes) / sizeof(kFlexes[0]))));
        break;

    case WandStep::Flex:
        wandStep_ = WandStep::Reveal;
        finishWandConfig();
        break;

    case WandStep::Reveal:
        break;
    }
}

void PrologueService::finishWandConfig()
{
    // 输出"魔杖选择你"的叙事 + 发出配置。
    emitNarrative(narrative(
        QStringLiteral("Mr. Ollivander"),
        QStringLiteral(
            "\xe2\x80\x9cYes\xe2\x80\xa6 yes, this is the one.\xe2\x80\x9d He presses "
            "the wand into your palm. <b>The wand blazes with a sudden golden light "
            "\xe2\x80\x94 warmth floods up your arm.</b> Sparks dance along its "
            "length like fireflies.<br><br>"
            "\xe2\x80\x9cCurious indeed how these things happen. The wand chose you, "
            "and you it. Treat it well.\xe2\x80\x9d<br><br>"
            "Your wand: <b>%1</b> wood, <b>%2</b> core, <b>%3\"</b>, <b>%4</b>.")
            .arg(config_.woodId,
                 config_.coreId,
                 QString::number(config_.lengthInches, 'f', 1),
                 config_.flexibility)));

    emit wandConfigured(config_);

    // 推进到 EXPRESS（本批到此暂停，后续实现）。
    stage_ = PrologueStage::Express;
    emit stageChanged(stage_);
    setScene(QStringLiteral("Platform 9\xc2\xbe \xe2\x80\x94 Coming Soon"),
             QStringLiteral("The Hogwarts Express awaits (next stage)"));
    emitChoices({});  // 清空选项栏
    emitNarrative(narrative(
        QStringLiteral("Narrator"),
        QStringLiteral(
            "<i>The prologue pauses here for now. The Hogwarts Express and Sorting "
            "Ceremony will continue in the next chapter.</i>")));
}

// ---------- 选项处理 ----------

void PrologueService::submitChoice(int index)
{
    if (stage_ == PrologueStage::Idle) return;

    if (stage_ == PrologueStage::DiagonAlleyPet) {
        const int n = static_cast<int>(sizeof(kPets) / sizeof(kPets[0]));
        if (index < 0 || index >= n) return;
        config_.petType = QString::fromLatin1(kPets[index].id);
        emitNarrative(narrative(
            QStringLiteral("Narrator"),
            QStringLiteral("You cradle your new <b>%1</b>. It seems pleased with its "
                           "choice of wizard. Diagon Alley awaits one last stop.")
                .arg(QString::fromLatin1(kPets[index].label))));
        enterWandStage();
        return;
    }

    if (stage_ == PrologueStage::DiagonAlleyWand) {
        switch (wandStep_) {
        case WandStep::Wood: {
            const int n = static_cast<int>(sizeof(kWoods) / sizeof(kWoods[0]));
            if (index < 0 || index >= n) return;
            config_.woodId = QString::fromLatin1(kWoods[index].id);
            advanceWandStep();
            return;
        }
        case WandStep::Core: {
            const int n = static_cast<int>(sizeof(kCores) / sizeof(kCores[0]));
            if (index < 0 || index >= n) return;
            config_.coreId = QString::fromLatin1(kCores[index].id);
            advanceWandStep();
            return;
        }
        case WandStep::Length: {
            const int n = static_cast<int>(sizeof(kLengths) / sizeof(kLengths[0]));
            if (index < 0 || index >= n) return;
            config_.lengthInches = kLengths[index];
            advanceWandStep();
            return;
        }
        case WandStep::Flex: {
            const int n = static_cast<int>(sizeof(kFlexes) / sizeof(kFlexes[0]));
            if (index < 0 || index >= n) return;
            config_.flexibility = QString::fromLatin1(kFlexes[index].id);
            advanceWandStep();
            return;
        }
        case WandStep::Reveal:
            return;
        }
    }

    // EXPRESS 阶段：输入"继续"可推进（但本批不做后续）。
    if (stage_ == PrologueStage::Express) {
        return;
    }
}

void PrologueService::submitText(const QString& text)
{
    // 自由文本输入：尝试解析为选项序号（1-based）。
    const QString t = text.trimmed();
    if (t.isEmpty()) return;
    bool ok = false;
    int n = t.toInt(&ok);
    if (ok && n >= 1) {
        submitChoice(n - 1);
    }
}

} // namespace arcane::application::prologue
