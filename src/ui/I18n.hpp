#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

namespace arcane::ui {

// 全局本地化单例。默认中文，提供中/英双语字典；切换语言时发出
// languageChanged 信号，所有需要重新翻译的 widget 监听并重排。
class I18n final : public QObject {
    Q_OBJECT

public:
    enum class Lang {
        Chinese,
        English,
    };
    Q_ENUM(Lang)

    static I18n& instance();

    void setLanguage(Lang lang);
    [[nodiscard]] Lang language() const noexcept { return m_lang_; }

    // 按 key 查表。找不到则原样返回 key 便于排错。
    [[nodiscard]] QString tr(const QString& key) const;

    // 带占位符 {0}/{1}/... 的翻译。
    [[nodiscard]] QString tr(const QString& key, const QStringList& args) const;

signals:
    void languageChanged(Lang lang);

private:
    I18n();
    void seed();
    void seedPair(const QString& key, const QString& zh, const QString& en);

    Lang m_lang_ = Lang::Chinese;
    QHash<QString, QPair<QString, QString>> map_;  // key -> (zh, en)
};

// TR：用于字符串字面量（"foo"）。用 QStringLiteral 让编译期生成 QString 数据。
#define TR(key) (::arcane::ui::I18n::instance().tr(QStringLiteral(key)))
// TR2：带占位符的版本（同 QStringLiteral 字面量）。
#define TR2(key, args) (::arcane::ui::I18n::instance().tr(QStringLiteral(key), args))
// TRQ：用于 QString 变量（不需要 QStringLiteral）。
#define TRQ(key) (::arcane::ui::I18n::instance().tr(key))

} // namespace arcane::ui
