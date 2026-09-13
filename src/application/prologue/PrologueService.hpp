#pragma once

#include "application/dto/CampusDTO.hpp"

#include <QObject>
#include <QString>
#include <QStringList>

namespace arcane::application::prologue {

// 开局剧情阶段。线性推进，不可回退。
enum class PrologueStage {
    Idle,             // 未开始
    DiagonAlleyPet,   // 阶段1：对角巷选购宠物
    DiagonAlleyWand,  // 阶段2：奥利凡德魔杖店
    Express,          // 阶段3：霍格沃茨特快（本批留接口，未实现）
    SortingHat,       // 阶段4：分院仪式（未实现）
    Completed,        // 全部完成，解锁自由探索
};

// 魔杖定制结果（阶段2结束时产出）。
struct WandConfig {
    QString petType;       // owl / cat / toad
    QString woodId;        // holly / yew / ...
    QString coreId;        // phoenix_feather / dragon_heartstring / unicorn_hair
    double lengthInches = 11.0;
    QString flexibility;   // Unyielding / Rigid / Supple / Pliant
};

// 单线状态机驱动的开局剧情。所有叙事/选项都通过信号推送到
// ChatEventWidget，禁止弹窗。本批实现阶段1（宠物）与阶段2（魔杖）。
class PrologueService final : public QObject {
    Q_OBJECT
public:
    explicit PrologueService(QObject* parent = nullptr);

    [[nodiscard]] PrologueStage stage() const noexcept { return stage_; }
    [[nodiscard]] bool isActive() const noexcept { return stage_ != PrologueStage::Idle && stage_ != PrologueStage::Completed; }

public slots:
    // 开始开局剧情（从宠物阶段起步）。
    void begin();
    // 玩家提交一个选项（按钮点击或输入框回车解析出的序号）。
    void submitChoice(int index);
    // 玩家在输入框输入自由文本（用于"继续"推进等）。
    void submitText(const QString& text);

signals:
    // 富文本叙事，追加到 ChatEventWidget。
    void narrativeProduced(const QString& htmlBody);
    // 当前场景 Header 标题。
    void sceneHeaderChanged(const QString& title);
    // 输入框 placeholder 提示。
    void inputHintChanged(const QString& hint);
    // 抛出一组选项按钮（玩家点其一推进）。空列表=隐藏选项栏。
    void choicesPresented(const QStringList& labels);
    // 阶段完成，携带收集到的数据（魔杖配置）。
    void wandConfigured(const WandConfig& config);
    // 整体进度变化。
    void stageChanged(PrologueStage stage);

private:
    // 内部子状态（阶段2有多个问答步骤）。
    enum class WandStep { Wood, Core, Length, Flex, Reveal };

    void enterPetStage();
    void enterWandStage();
    void advanceWandStep();
    void finishWandConfig();

    void emitNarrative(const QString& html);
    void emitChoices(const QStringList& labels);
    void setScene(const QString& title, const QString& hint);

    PrologueStage stage_ = PrologueStage::Idle;
    WandStep wandStep_ = WandStep::Wood;
    WandConfig config_;
};

} // namespace arcane::application::prologue
