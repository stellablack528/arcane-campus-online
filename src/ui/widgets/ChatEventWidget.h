#pragma once

#include <QString>
#include <QWidget>

class QComboBox;
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

public slots:
    void appendMessage(const QString &channel, const QString &speaker, const QString &text);

private slots:
    void onSendClicked();

private:
    void buildUi();
    void seedMessages();

    QTextEdit *m_eventView = nullptr;
    QComboBox *m_channelCombo = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_sendButton = nullptr;
};
