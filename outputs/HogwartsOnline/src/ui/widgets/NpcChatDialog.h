#pragma once

#include <QDialog>
#include <QList>
#include <QPair>
#include <QString>

class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QTimer;
class DeepSeekClient;
struct NpcPersonality;

class NpcChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NpcChatDialog(DeepSeekClient *client,
                           const NpcPersonality &personality,
                           QWidget *parent = nullptr);
    void retranslateUi();

signals:
    void dialogClosed(const QString &npcId);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onSendClicked();
    void onReplyReceived(const QString &npcId, const QString &reply, bool ok);

private:
    void buildUi();
    void appendBubble(const QString &speaker, const QString &text, bool isNpc);
    QString systemPrompt() const;

    DeepSeekClient *m_client = nullptr;
    NpcPersonality m_personality;

    QLabel *m_titleLabel = nullptr;
    QLabel *m_houseLabel = nullptr;
    QTextEdit *m_chatView = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_sendButton = nullptr;
    QPushButton *m_apiKeyButton = nullptr;

    // 对话历史
    QList<QPair<QString, QString>> m_history;  // (role, content) — "user" / "assistant"
    bool m_waiting = false;
};
