#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;

namespace arcane::core {

// Calls the DeepSeek chat-completions API (OpenAI-compatible) to produce
// NPC and story replies. The API key is never hardcoded: it is read from the
// DEEPSEEK_API_KEY process environment or local QSettings.
class DeepSeekClient final : public QObject {
    Q_OBJECT

public:
    explicit DeepSeekClient(QObject* parent = nullptr);

    // Returns true when a key is available from either local source.
    [[nodiscard]] static bool hasApiKey();

public slots:
    // Sends a chat request. The channel/speaker context is echoed back in
    // replyReceived so the caller knows which conversation the answer belongs to.
    void sendChat(const QString& systemPrompt,
                  const QString& userMessage,
                  const QString& channel,
                  const QString& speaker);

signals:
    void replyReceived(const QString& channel, const QString& speaker, const QString& text);
    void errorOccurred(const QString& message);

private:
    [[nodiscard]] static QString loadApiKey();

    QNetworkAccessManager* networkManager_;
};

} // namespace arcane::core
