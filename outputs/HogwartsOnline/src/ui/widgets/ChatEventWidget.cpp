#include "ui/widgets/ChatEventWidget.h"
#include "core/LanguageManager.h"

#include <QComboBox>
#include <QDateTime>
#include <QEvent>
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
    retranslateUi();
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

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("PanelTitle");

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
    m_input = new QLineEdit(this);
    m_sendButton = new QPushButton(this);
    m_sendButton->setObjectName("PrimaryButton");

    inputLayout->addWidget(m_channelCombo);
    inputLayout->addWidget(m_input, 1);
    inputLayout->addWidget(m_sendButton);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_eventView, 1);
    layout->addLayout(inputLayout);

    connect(m_sendButton, &QPushButton::clicked, this, &ChatEventWidget::onSendClicked);
    connect(m_input, &QLineEdit::returnPressed, this, &ChatEventWidget::onSendClicked);
    connect(m_channelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChatEventWidget::onChannelChanged);
}

void ChatEventWidget::seedMessages()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    if (isCn) {
        m_eventView->append("<p><b>[系统]</b><br>早餐时间已开始。<br>礼堂现已开放。</p>");
        m_eventView->append("<p><b>麦格教授:</b><br>变形课十五分钟后开始。<br>地点：变形课教室。</p>");
        m_eventView->append("<p><b>[图书馆]</b> <b>赫敏·格兰杰:</b><br>今天的图书馆异常安静。</p>");
        m_eventView->append("<p><span class='time'>08:55</span> 加雷斯·韦斯莱 进入了变形课教室。</p>");
        m_eventView->append("<p><span class='time'>09:03</span> 麦格教授 开始上课。</p>");
    } else {
        m_eventView->append("<p><b>[System]</b><br>Breakfast time has started.<br>Great Hall is now open.</p>");
        m_eventView->append("<p><b>Professor McGonagall:</b><br>Transfiguration class begins in fifteen minutes.<br>Location: Transfiguration Classroom.</p>");
        m_eventView->append("<p><b>[Library]</b> <b>Hermione Granger:</b><br>The library is unusually quiet today.</p>");
        m_eventView->append("<p><span class='time'>08:55</span> Gareth Weasley entered the Transfiguration Classroom.</p>");
        m_eventView->append("<p><span class='time'>09:03</span> Professor McGonagall started the lesson.</p>");
    }
}

void ChatEventWidget::onSendClicked()
{
    const QString text = m_input->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);
    const QString channel = m_channelCombo->currentData().toString(); // 英文 channel id
    const QString speaker = isCn ? "我" : "You";
    emit sendChatRequested(channel, text);
    appendMessage(channel, speaker, text);
    m_input->clear();
}

void ChatEventWidget::onChannelChanged()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);
    QString channelId = m_channelCombo->currentData().toString();
    QString placeText;
    if (isCn) {
        if (channelId == "location") placeText = "向当前场景频道发言...";
        else if (channelId == "world") placeText = "向世界频道发言...";
        else if (channelId == "party") placeText = "向队伍频道发言...";
        else placeText = "发言...";
    } else {
        if (channelId == "location") placeText = "Write to the current campus channel...";
        else if (channelId == "world") placeText = "Write to the world channel...";
        else if (channelId == "party") placeText = "Write to your party channel...";
        else placeText = "Type a message...";
    }
    m_input->setPlaceholderText(placeText);
}

void ChatEventWidget::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    m_titleLabel->setText(isCn ? "校园聊天与事件" : "Campus Chat & Events");
    m_sendButton->setText(isCn ? "发送" : "Send");

    // 频道下拉（保留英文 id 作为 data，中文作为显示）
    int prevIdx = m_channelCombo->currentIndex();
    m_channelCombo->clear();
    m_channelCombo->addItem(isCn ? "📍 场景" : "📍 Location", "location");
    m_channelCombo->addItem(isCn ? "🌍 世界" : "🌍 World", "world");
    m_channelCombo->addItem(isCn ? "👥 队伍" : "👥 Party", "party");
    if (prevIdx >= 0 && prevIdx < m_channelCombo->count())
        m_channelCombo->setCurrentIndex(prevIdx);

    onChannelChanged();
}

void ChatEventWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
