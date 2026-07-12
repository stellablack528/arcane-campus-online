#include "ui/widgets/ChatEventWidget.h"

#include <QComboBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QTextDocument>
#include <QTime>
#include <QVBoxLayout>

ChatEventWidget::ChatEventWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("ChatEventWidget");
    buildUi();
    seedMessages();
}

void ChatEventWidget::appendMessage(const QString &channel, const QString &speaker, const QString &text)
{
    const QString time = QTime::currentTime().toString("hh:mm");
    m_eventView->append(QString("<p><span class='time'>%1</span> <b>[%2]</b> <b>%3:</b><br>%4</p>")
                            .arg(time, channel.toHtmlEscaped(), speaker.toHtmlEscaped(), text.toHtmlEscaped()));
}

void ChatEventWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *title = new QLabel("Campus Chat & Events", this);
    title->setObjectName("PanelTitle");

    m_eventView = new QTextEdit(this);
    m_eventView->setObjectName("ParchmentView");
    m_eventView->setReadOnly(true);
    m_eventView->document()->setDefaultStyleSheet(
        "p { margin: 10px 0; color: #2a1e14; }"
        ".time { color: #6b4d2e; }"
        "b { color: #3b2416; }"
    );

    auto *inputLayout = new QHBoxLayout;
    inputLayout->setSpacing(8);

    m_channelCombo = new QComboBox(this);
    m_channelCombo->addItems({"Location", "World", "Party"});

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("Write to the current campus channel...");

    m_sendButton = new QPushButton("Send", this);
    m_sendButton->setObjectName("PrimaryButton");

    inputLayout->addWidget(m_channelCombo);
    inputLayout->addWidget(m_input, 1);
    inputLayout->addWidget(m_sendButton);

    layout->addWidget(title);
    layout->addWidget(m_eventView, 1);
    layout->addLayout(inputLayout);

    connect(m_sendButton, &QPushButton::clicked, this, &ChatEventWidget::onSendClicked);
    connect(m_input, &QLineEdit::returnPressed, this, &ChatEventWidget::onSendClicked);
}

void ChatEventWidget::seedMessages()
{
    m_eventView->append("<p><b>[System]</b><br>Breakfast time has started.<br>Great Hall is now open.</p>");
    m_eventView->append("<p><b>Professor McGonagall:</b><br>Transfiguration class begins in fifteen minutes.<br>Location: Transfiguration Classroom.</p>");
    m_eventView->append("<p><b>[Library]</b> <b>Hermione Granger:</b><br>The library is unusually quiet today.</p>");
    m_eventView->append("<p><span class='time'>08:55</span> Gareth Weasley entered the Transfiguration Classroom.</p>");
    m_eventView->append("<p><span class='time'>09:03</span> Professor McGonagall started the lesson.</p>");
}

void ChatEventWidget::onSendClicked()
{
    const QString text = m_input->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    const QString channel = m_channelCombo->currentText();
    emit sendChatRequested(channel, text);
    appendMessage(channel, "You", text);
    m_input->clear();
}
