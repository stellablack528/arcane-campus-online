#pragma once

#include <QString>
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QLabel;

class ChatEventWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatEventWidget(QWidget *parent = nullptr);
    void retranslateUi();

signals:
    void sendChatRequested(const QString &channel, const QString &text);
    void npcDialogueRequested(const QString &npcId);
    void systemEventClicked(const QString &eventId);

public slots:
    void appendMessage(const QString &channel, const QString &speaker, const QString &text);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onSendClicked();
    void onChannelChanged();

private:
    void buildUi();
    void seedMessages();

    QLabel *m_titleLabel = nullptr;
    QTextEdit *m_eventView = nullptr;
    QComboBox *m_channelCombo = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_sendButton = nullptr;
};
