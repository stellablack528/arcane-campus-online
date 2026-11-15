#include "application/world/EraWorldPack.hpp"

#include <storyforge/io/story_loader.hpp>

#include <QFile>

namespace arcane::application::world {

std::shared_ptr<const storyforge::StoryPackage> EraWorldPack::load(const QString& eraId,
                                                                  QString* error)
{
    if (eraId != QLatin1String("parents") && eraId != QLatin1String("harry")
        && eraId != QLatin1String("riddle")) {
        if (error) *error = QStringLiteral("未知的入学时代。");
        return {};
    }
    QFile file(QStringLiteral(":/story/worlds/%1.json").arg(eraId));
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = QStringLiteral("世界包文件缺失：%1").arg(eraId);
        return {};
    }
    const auto loaded = storyforge::load_story_package_text(file.readAll().toStdString());
    if (!loaded.package || !loaded.report.valid()) {
        if (error) *error = loaded.report.issues.empty()
            ? QStringLiteral("世界包无法加载。")
            : QString::fromStdString(loaded.report.issues.front().message);
        return {};
    }
    if (loaded.package->id.value() != "arcane-campus-" + eraId.toStdString()) {
        if (error) *error = QStringLiteral("世界包与入学时代不匹配。");
        return {};
    }
    return std::make_shared<const storyforge::StoryPackage>(*loaded.package);
}

QString EraWorldPack::title(const storyforge::StoryPackage& package)
{
    return QString::fromStdString(package.title);
}

QString EraWorldPack::classmate(const storyforge::StoryPackage& package)
{
    for (const auto& npc : package.agent_npcs)
        if (npc.id.value() == "npc.classmate") return QString::fromStdString(npc.name);
    return {};
}

QString EraWorldPack::professor(const storyforge::StoryPackage& package)
{
    for (const auto& npc : package.agent_npcs)
        if (npc.id.value() == "npc.professor") return QString::fromStdString(npc.name);
    return {};
}

} // namespace arcane::application::world
