#pragma once

#include "application/prologue/WandLoreService.hpp"

#include <QObject>
#include <QString>
#include <QStringList>

namespace arcane::application::prologue {

enum class PrologueStage {
    Idle,
    DiagonAlleyArrival,
    Ollivanders,
    WandReading,
    DiagonAlleyOpen,
    Completed,
};

// A deliberately small, complete first-year chapter. Later chapters can be
// appended without reopening character creation or the wand ceremony.
class PrologueService final : public QObject {
    Q_OBJECT

public:
    explicit PrologueService(QObject* parent = nullptr);

    [[nodiscard]] PrologueStage stage() const noexcept { return stage_; }
    [[nodiscard]] bool isActive() const noexcept;

    // Starts at the entrance to Diagon Alley. The selected wand stays sealed in
    // story state until the player completes Ollivander's ceremony.
    void begin(const WandConfig& selectedWand);
    // Re-enters the last safe scene after a local save is loaded.
    void resume(const WandConfig& selectedWand, PrologueStage savedStage);

public slots:
    void submitChoice(int index);
    void submitText(const QString& text);

signals:
    void narrativeProduced(const QString& speaker, const QString& htmlBody);
    void sceneHeaderChanged(const QString& title);
    void inputHintChanged(const QString& hint);
    void choicesPresented(const QStringList& labels);
    void wandConfigured(const WandConfig& config);
    void stageChanged(PrologueStage stage);

private:
    void enterDiagonAlley();
    void enterOllivanders();
    void revealWand();
    void finishWandReading(const QString& reading, bool generatedByAi);
    void completeChapter();
    void setStage(PrologueStage stage);
    void setScene(const QString& title, const QString& hint);

    PrologueStage stage_ = PrologueStage::Idle;
    WandConfig selectedWand_;
    WandLoreService loreService_;
};

} // namespace arcane::application::prologue
