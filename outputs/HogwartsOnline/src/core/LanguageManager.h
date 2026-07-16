#pragma once

#include <QObject>
#include <QString>

// 语言枚举
enum class Language {
    English = 0,
    Chinese = 1
};

// 全局语言管理器（单例）
class LanguageManager : public QObject
{
    Q_OBJECT

public:
    static LanguageManager &instance();

    Language currentLanguage() const { return m_language; }
    QString currentCode() const;     // "en_US" / "zh_CN"
    QString currentDisplayName() const;

    // 切换语言（发出 languageChanged 信号）
    void setLanguage(Language lang);

    // 辅助：双语文本查找
    QString tr(const QString &english, const QString &chinese) const;

signals:
    void languageChanged(Language newLanguage);

private:
    explicit LanguageManager(QObject *parent = nullptr);
    Language m_language = Language::English;
};
