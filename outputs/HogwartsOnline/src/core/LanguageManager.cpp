#include "core/LanguageManager.h"

LanguageManager &LanguageManager::instance()
{
    static LanguageManager instance;
    return instance;
}

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
{
}

QString LanguageManager::currentCode() const
{
    return m_language == Language::Chinese ? QStringLiteral("zh_CN")
                                           : QStringLiteral("en_US");
}

QString LanguageManager::currentDisplayName() const
{
    return m_language == Language::Chinese ? QStringLiteral("中文")
                                           : QStringLiteral("English");
}

void LanguageManager::setLanguage(Language lang)
{
    if (m_language == lang) {
        return;
    }
    m_language = lang;
    emit languageChanged(lang);
}

QString LanguageManager::tr(const QString &english, const QString &chinese) const
{
    return m_language == Language::Chinese ? chinese : english;
}
