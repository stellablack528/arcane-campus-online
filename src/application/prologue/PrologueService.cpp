#include "application/prologue/PrologueService.hpp"

#include "ui/I18n.hpp"

namespace arcane::application::prologue {

namespace {

bool isChinese()
{
    return arcane::ui::I18n::instance().language() == arcane::ui::I18n::Lang::Chinese;
}

QString localized(const char* chinese, const char* english)
{
    return isChinese() ? QString::fromUtf8(chinese) : QString::fromUtf8(english);
}

QString safeParagraph(QString text)
{
    return text.toHtmlEscaped().replace(QStringLiteral("\n"), QStringLiteral("<br>"));
}

} // namespace

PrologueService::PrologueService(QObject* parent)
    : QObject(parent)
    , loreService_(this)
{
    connect(&loreService_, &WandLoreService::readingReady,
            this, &PrologueService::finishWandReading);
}

bool PrologueService::isActive() const noexcept
{
    return stage_ != PrologueStage::Idle && stage_ != PrologueStage::Completed;
}

void PrologueService::begin(const WandConfig& selectedWand)
{
    selectedWand_ = selectedWand;
    enterDiagonAlley();
}

void PrologueService::resume(const WandConfig& selectedWand, PrologueStage savedStage)
{
    selectedWand_ = selectedWand;
    switch (savedStage) {
    case PrologueStage::Ollivanders:
        enterOllivanders();
        break;
    case PrologueStage::WandReading:
        revealWand();
        break;
    case PrologueStage::DiagonAlleyArrival:
    case PrologueStage::Idle:
        enterDiagonAlley();
        break;
    case PrologueStage::DiagonAlleyOpen:
    case PrologueStage::Completed:
        setStage(PrologueStage::Completed);
        break;
    }
}

void PrologueService::setStage(PrologueStage stage)
{
    stage_ = stage;
    emit stageChanged(stage_);
}

void PrologueService::setScene(const QString& title, const QString& hint)
{
    emit sceneHeaderChanged(title);
    emit inputHintChanged(hint);
}

void PrologueService::enterDiagonAlley()
{
    setStage(PrologueStage::DiagonAlleyArrival);
    setScene(localized("对角巷 · 新生采购日", "Diagon Alley · First-Year Shopping"),
             localized("先去取得你的魔杖……", "First, claim your wand..."));

    const QString body = isChinese()
        ? QStringLiteral(
            "砖墙在身后合拢，午后的光落在蜿蜒的鹅卵石街面上。猫头鹰的叫声、坩埚碰撞声与商贩的吆喝混在一起——你第一次真正站进了魔法世界。<br><br>"
            "左侧的<b>丽痕书店</b>出售课本、地图和会自己改错的羽毛笔；<b>摩金夫人长袍专卖店</b>正在给新生量尺寸；"
            "前方的<b>药材店</b>堆满干燥草药、魔药材料与黄铜坩埚；<b>神奇动物商店</b>里传来猫、蟾蜍和猫头鹰的动静。<br><br>"
            "而街道深处那块褪色的金字招牌写着：<b>奥利凡德：自公元前382年即制作精良魔杖</b>。你的第一站已经决定。")
        : QStringLiteral(
            "The brick wall seals behind you, and afternoon light spills across the winding cobbles. Owl calls, clattering cauldrons, and shopkeepers' cries mingle around you—your first real step into the wizarding world.<br><br>"
            "<b>Flourish and Blotts</b> sells textbooks, maps, and self-correcting quills; <b>Madam Malkin's</b> is measuring first-years for robes. "
            "Ahead, the <b>Apothecary</b> offers dried herbs, potion ingredients, and brass cauldrons, while cats, toads, and owls stir inside the <b>Magical Menagerie</b>.<br><br>"
            "Farther down the street, a faded gold sign reads: <b>Ollivanders: Makers of Fine Wands since 382 B.C.</b> Your first stop is already decided.");

    emit narrativeProduced(localized("旁白", "Narrator"), body);
    emit choicesPresented({localized("走进奥利凡德魔杖店", "Enter Ollivanders")});
}

void PrologueService::enterOllivanders()
{
    setStage(PrologueStage::Ollivanders);
    setScene(localized("奥利凡德魔杖店 · 尘封的长匣", "Ollivanders · Dusty Wand Boxes"),
             localized("让魔杖作出选择……", "Let the wand make its choice..."));

    const QString body = isChinese()
        ? QStringLiteral(
            "门铃轻响。狭长店铺的墙壁从地板一直堆到天花板，全是覆着薄尘的细长盒子。<br><br>"
            "奥利凡德先生从货架间无声地出现，苍白的眼睛在你与那张提前送来的定制单之间移动。"
            "“木材、杖芯、长度、柔韧度……这些只是让我们找到它的线索。”他抽出一只盒子，吹去盒盖上的灰。"
            "“真正的答案，要由魔杖来给。”<br><br>"
            "盒中正躺着你在入学登记时选定的那根魔杖。")
        : QStringLiteral(
            "A bell whispers above the door. The narrow shop is stacked floor to ceiling with slender boxes filmed in dust.<br><br>"
            "Mr. Ollivander appears silently between the shelves, pale eyes moving from you to the order slip sent ahead. "
            "\"Wood, core, length, flexibility... merely clues that help us find it.\" He draws out one box and blows the dust from its lid. "
            "\"The wand must give us the true answer.\"<br><br>"
            "Inside lies the wand you specified during enrollment.");

    emit narrativeProduced(localized("奥利凡德先生", "Mr. Ollivander"), body);
    emit choicesPresented({localized("伸手握住魔杖", "Take the wand")});
}

void PrologueService::revealWand()
{
    setStage(PrologueStage::WandReading);
    emit choicesPresented({});
    setScene(localized("奥利凡德魔杖店 · 魔杖的回答", "Ollivanders · The Wand's Answer"),
             localized("金色火花正在消散……", "Golden sparks are settling..."));

    const QString body = isChinese()
        ? QStringLiteral(
            "你的手指刚刚合拢，杖尖便迸出一束温暖的金色火花。它们沿着天花板旋转一周，照亮成百上千只沉睡的魔杖盒，最后像萤火一样落在你的肩头。<br><br>"
            "“就是它。”奥利凡德先生的嘴角露出一丝笑意。“不是你挑中了它——是它认出了你。”<br><br>"
            "你的魔杖：<b>%1</b>木，<b>%2</b>杖芯，<b>%3 英寸</b>，<b>%4</b>。")
              .arg(selectedWand_.woodName.toHtmlEscaped(),
                   selectedWand_.coreName.toHtmlEscaped(),
                   QString::number(selectedWand_.lengthInches, 'f', 2),
                   selectedWand_.flexibility.toHtmlEscaped())
        : QStringLiteral(
            "The instant your fingers close, warm golden sparks burst from the tip. They circle the ceiling, waking hundreds of sleeping wand boxes, then settle over your shoulders like fireflies.<br><br>"
            "\"This is the one.\" A faint smile touches Mr. Ollivander's face. \"You did not choose it—it recognised you.\"<br><br>"
            "Your wand: <b>%1</b> wood, <b>%2</b> core, <b>%3 inches</b>, <b>%4</b>.")
              .arg(selectedWand_.woodName.toHtmlEscaped(),
                   selectedWand_.coreName.toHtmlEscaped(),
                   QString::number(selectedWand_.lengthInches, 'f', 2),
                   selectedWand_.flexibility.toHtmlEscaped());

    emit narrativeProduced(localized("旁白", "Narrator"), body);
    emit wandConfigured(selectedWand_);
    emit narrativeProduced(
        localized("奥利凡德先生", "Mr. Ollivander"),
        localized("奥利凡德先生端详着仍在发光的杖尖，像是在听一段只有他能听见的低语……",
                  "Mr. Ollivander studies the glowing tip as though listening to a whisper meant for him alone..."));
    loreService_.requestReading(selectedWand_);
}

void PrologueService::finishWandReading(const QString& reading, bool generatedByAi)
{
    if (stage_ != PrologueStage::WandReading) {
        return;
    }

    emit narrativeProduced(
        localized("奥利凡德先生", "Mr. Ollivander"),
        safeParagraph(reading));

    setStage(PrologueStage::DiagonAlleyOpen);
    setScene(localized("对角巷 · 新世界仍在展开", "Diagon Alley · A World Still Unfolding"),
             localized("收好魔杖，继续你的采购……", "Pocket your wand and continue shopping..."));

    const QString sourceNote = generatedByAi
        ? localized("那段判断像刚从魔杖的火花里生长出来。", "The reading seems to have grown from the wand's own sparks.")
        : localized("店内的古老魔杖档案替它留下了这段判断。", "The shop's old wand archive supplies the reading.");
    const QString body = isChinese()
        ? QStringLiteral(
            "%1<br><br>你带着第一件真正属于自己的魔法物品重新踏上街道。丽痕书店、摩金夫人长袍店、药材店与神奇动物商店仍然开着门；"
            "远处还有更多招牌在雾蒙蒙的玻璃后闪烁。<br><br><b>新生创建与魔杖领取已经完成。</b>从此刻起，你可以继续探索对角巷，之后的每一家店都能独立接入新的剧情。")
              .arg(sourceNote)
        : QStringLiteral(
            "%1<br><br>You step back onto the street carrying the first magical object that is truly yours. Flourish and Blotts, Madam Malkin's, the Apothecary, and the Magical Menagerie still have their doors open; "
            "more signs glimmer behind misted windows farther along.<br><br><b>Character creation and wand collection are complete.</b> From here, Diagon Alley is open for exploration, and each remaining shop can become its own chapter.")
              .arg(sourceNote);
    emit narrativeProduced(localized("旁白", "Narrator"), body);
    emit choicesPresented({localized("收好魔杖，继续探索对角巷", "Pocket the wand and explore Diagon Alley")});
}

void PrologueService::completeChapter()
{
    setStage(PrologueStage::Completed);
    emit choicesPresented({});
    setScene(localized("对角巷 · 自由探索", "Diagon Alley · Free Exploration"),
             localized("描述你接下来想做什么……", "Describe what you want to do next..."));
    emit narrativeProduced(
        localized("世界", "World"),
        localized("对角巷不再只是一段引导剧情。街上的每扇门、每道声音，现在都可以成为你的下一步。",
                  "Diagon Alley is no longer only an introduction. Every door and every voice can now become your next step."));
}

void PrologueService::submitChoice(int index)
{
    if (index != 0) {
        return;
    }

    switch (stage_) {
    case PrologueStage::DiagonAlleyArrival:
        enterOllivanders();
        break;
    case PrologueStage::Ollivanders:
        revealWand();
        break;
    case PrologueStage::DiagonAlleyOpen:
        completeChapter();
        break;
    default:
        break;
    }
}

void PrologueService::submitText(const QString& text)
{
    bool ok = false;
    const int choice = text.trimmed().toInt(&ok);
    if (ok && choice >= 1) {
        submitChoice(choice - 1);
    }
}

} // namespace arcane::application::prologue
