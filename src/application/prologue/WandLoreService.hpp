#pragma once

#include <QObject>
#include <QString>

#include <memory>

namespace arcane::core {
class DeepSeekClient;
}

namespace arcane::application::prologue {

// The physical wand selected during enrollment. Story ownership is granted only
// after Ollivander's reveal, so the creation dialog never spoils the ceremony.
struct WandConfig {
    QString woodId;
    QString coreId;
    QString woodName;
    QString coreName;
    double lengthInches = 11.0;
    QString flexibility;
    QString handleStyle;
    QString finish;
    QString colorTone;
    QString engraving;
};

// Produces a short personality reading for a wand. DeepSeek is an optional
// enhancement: missing credentials or request failures fall back to the bundled
// wand archive so the prologue can always finish.
class WandLoreService final : public QObject {
    Q_OBJECT

public:
    explicit WandLoreService(QObject* parent = nullptr);
    ~WandLoreService() override;

    void requestReading(const WandConfig& wand);

signals:
    void readingReady(const QString& reading, bool generatedByAi);

private:
    [[nodiscard]] QString localReading(const WandConfig& wand) const;

    std::unique_ptr<core::DeepSeekClient> client_;
    WandConfig pendingWand_;
    bool requestPending_ = false;
};

} // namespace arcane::application::prologue
