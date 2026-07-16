#include "core/LanguageManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QTranslator>

LanguageManager &LanguageManager::instance()
{
    static LanguageManager instance;
    return instance;
}

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
    , m_translator(new QTranslator(this))
{
    // 预先加载一次英文（作为默认）
}

QString LanguageManager::currentCode() const
{
    return m_language == Language::Chinese ? QStringLiteral("zh_CN")
                                           : QStringLiteral("en_US");
}

void LanguageManager::setLanguage(Language lang)
{
    if (m_language == lang) {
        return;
    }
    m_language = lang;
    loadTranslation(lang);
    emit languageChanged(lang);
}

void LanguageManager::loadTranslation(Language lang)
{
    // 先移除旧翻译
    if (m_translator) {
        qApp->removeTranslator(m_translator);
    }

    // 英文不需要翻译
    if (lang == Language::English) {
        return;
    }

    // 尝试加载 .qm 翻译文件
    // 优先查找当前目录，其次查找 i18n 目录
    const QString qmFile = QStringLiteral("arcane_campus_zh_CN");
    QStringList searchPaths = {
        QStringLiteral("."),
        QStringLiteral("i18n"),
        QStringLiteral(":/i18n"),            // Qt 资源文件内
        QCoreApplication::applicationDirPath(),
        QCoreApplication::applicationDirPath() + "/i18n",
    };

    bool loaded = false;
    for (const auto &path : searchPaths) {
        if (m_translator->load(qmFile, path)) {
            loaded = true;
            break;
        }
    }

    if (loaded) {
        qApp->installTranslator(m_translator);
    }
    // 如果 .qm 未找到，降级使用 LanguageManager::tr() 内联翻译
}

QString LanguageManager::tr(const QString &english, const QString &chinese) const
{
    return m_language == Language::Chinese ? chinese : english;
}
