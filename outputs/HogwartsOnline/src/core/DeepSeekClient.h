#pragma once

#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QJsonObject>

// -------------- NPC 人设 ---------------
struct NpcPersonality {
    QString npcId;           // 内部 ID
    QString displayNameEn;   // 英文名
    QString displayNameCn;   // 中文名
    QString systemPromptEn;  // 英文系统提示词
    QString systemPromptCn;  // 中文系统提示词
    QString house;           // 学院
    QString role;            // 身份
    QString greetingEn;      // 首次对话问候（英文）
    QString greetingCn;      // 首次对话问候（中文）
};

// -------------- DeepSeek API 客户端 ---------------
class DeepSeekClient : public QObject
{
    Q_OBJECT

public:
    explicit DeepSeekClient(QObject *parent = nullptr);

    // 设置 API Key
    void setApiKey(const QString &key);
    QString apiKey() const { return m_apiKey; }

    // 获取所有预置 NPC
    static QList<NpcPersonality> presetNpcs();

    // 发送对话请求
    void sendMessage(const QString &npcId,
                     const QString &systemPrompt,
                     const QString &message,
                     const QList<QPair<QString, QString>> &history);

signals:
    void replyReceived(const QString &npcId, const QString &reply, bool ok);
    void streamingChunk(const QString &npcId, const QString &chunk);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QString m_apiKey;
    QNetworkAccessManager *m_nam = nullptr;
    QMap<QNetworkReply *, QString> m_pendingNpcIds;
};
