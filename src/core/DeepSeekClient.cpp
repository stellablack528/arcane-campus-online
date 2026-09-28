#include "core/DeepSeekClient.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <QtGlobal>

namespace arcane::core {

namespace {
constexpr const char* kApiUrl = "https://api.deepseek.com/chat/completions";
constexpr const char* kModelName = "deepseek-chat";
constexpr const char* kSettingsKey = "ai/deepseek_api_key";
} // namespace

DeepSeekClient::DeepSeekClient(QObject* parent)
    : QObject(parent)
    , networkManager_(new QNetworkAccessManager(this))
{
}

bool DeepSeekClient::hasApiKey()
{
    return !loadApiKey().isEmpty();
}

QString DeepSeekClient::loadApiKey()
{
    // Environment configuration is convenient for development and CI, while
    // QSettings supports the in-app settings screen. Neither source is tracked
    // by Git, and the key is read afresh for every request.
    const QString environmentKey = QString::fromUtf8(qgetenv("DEEPSEEK_API_KEY")).trimmed();
    if (!environmentKey.isEmpty()) {
        return environmentKey;
    }
    QSettings settings(QStringLiteral("FanProject"), QStringLiteral("Hogwarts Online"));
    return settings.value(QLatin1String(kSettingsKey)).toString().trimmed();
}

void DeepSeekClient::sendChat(const QString& systemPrompt,
                              const QString& userMessage,
                              const QString& channel,
                              const QString& speaker)
{
    const QString apiKey = loadApiKey();
    if (apiKey.isEmpty()) {
        emit errorOccurred(QStringLiteral("No AI API key configured. Set it via System → AI Settings."));
        return;
    }

    QJsonObject body;
    body.insert(QStringLiteral("model"), QLatin1String(kModelName));
    QJsonArray messages;
    QJsonObject systemMessage;
    systemMessage.insert(QStringLiteral("role"), QStringLiteral("system"));
    systemMessage.insert(QStringLiteral("content"), systemPrompt);
    QJsonObject userEntry;
    userEntry.insert(QStringLiteral("role"), QStringLiteral("user"));
    userEntry.insert(QStringLiteral("content"), userMessage);
    messages.append(systemMessage);
    messages.append(userEntry);
    body.insert(QStringLiteral("messages"), messages);

    QNetworkRequest request{QUrl(QLatin1String(kApiUrl))};
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());

    QNetworkReply* reply = networkManager_->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    // A slow or unreachable API must never trap a story scene indefinitely.
    QTimer::singleShot(15000, reply, [reply] {
        if (reply->isRunning()) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, channel, speaker] {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString message = QStringLiteral("AI request failed (HTTP %1): %2")
                                  .arg(status)
                                  .arg(reply->errorString());
            if (status == 401) {
                message = QStringLiteral("AI request rejected (401): your API key looks invalid. "
                                         "Check it in System → AI Settings.");
            }
            emit errorOccurred(message);
            return;
        }

        const auto document = QJsonDocument::fromJson(reply->readAll());
        const auto choices = document.object().value(QStringLiteral("choices")).toArray();
        if (choices.isEmpty()) {
            emit errorOccurred(QStringLiteral("AI reply was empty."));
            return;
        }
        const auto content = choices.first().toObject()
                                 .value(QStringLiteral("message"))
                                 .toObject()
                                 .value(QStringLiteral("content"))
                                 .toString();

        if (content.isEmpty()) {
            emit errorOccurred(QStringLiteral("AI reply was empty."));
            return;
        }
        emit replyReceived(channel, speaker, content.trimmed());
    });
}

} // namespace arcane::core
