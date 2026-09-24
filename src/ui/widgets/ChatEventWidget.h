#pragma once

#include <QString>
#include <QWidget>

class QComboBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;

class ChatEventWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatEventWidget(QWidget *parent = nullptr);

signals:
    void sendChatRequested(const QString &channel, const QString &text);
    void npcDialogueRequested(const QString &npcId);
    void systemEventClicked(const QString &eventId);
    // 玩家点击了某个选项按钮（开局剧情用），index 从 0 起。
    void choiceSelected(int index);

public slots:
    void appendMessage(const QString &channel, const QString &speaker, const QString &text);
    // 追加富文本消息：channel/speaker 会转义，htmlBody 原样插入（供学院色日志等可信内容）。
    void appendRichMessage(const QString &channel, const QString &speaker, const QString &htmlBody);
    // 设置场景顶部 Header 标题。
    void setSceneHeader(const QString &title);
    // 设置输入框 placeholder。
    void setInputHint(const QString &hint);
    // 抛出一组选项按钮（空列表=隐藏选项栏）。
    void presentChoices(const QStringList &labels);

private slots:
    void onSendClicked();

private:
    enum class MessageKind {
        Narration,
        NpcDialogue,
        PlayerDialogue,
        WorldEvent,
        SystemNotice,
    };

    void buildUi();
    void seedMessages();
    void retranslateUi();
    void clearChoices();
    [[nodiscard]] MessageKind classifyMessage(const QString &channel,
                                              const QString &speaker) const;
    void appendEntry(MessageKind kind,
                     const QString &channel,
                     const QString &speaker,
                     const QString &body,
                     bool bodyIsTrustedHtml = false);

    QTextEdit *m_eventView = nullptr;
    QComboBox *m_channelCombo = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_sendButton = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_sceneHeaderLabel = nullptr;
    QWidget *m_choiceBar = nullptr;
    QHBoxLayout *m_choiceLayout = nullptr;
    QStringList m_channelLabels;
};
