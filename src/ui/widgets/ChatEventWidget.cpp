#include "ui/widgets/ChatEventWidget.h"

#include "ui/I18n.hpp"

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

void ChatEventWidget::appendRichMessage(const QString &channel, const QString &speaker, const QString &htmlBody)
{
    const QString time = QTime::currentTime().toString("hh:mm");
    m_eventView->append(QString("<p><span class='time'>%1</span> <b>[%2]</b> <b>%3:</b><br>%4</p>")
                            .arg(time, channel.toHtmlEscaped(), speaker.toHtmlEscaped(), htmlBody));
}

void ChatEventWidget::setSceneHeader(const QString &title)
{
    if (m_sceneHeaderLabel) {
        if (title.isEmpty()) {
            m_sceneHeaderLabel->hide();
        } else {
            m_sceneHeaderLabel->setText(title);
            m_sceneHeaderLabel->show();
        }
    }
}

void ChatEventWidget::setInputHint(const QString &hint)
{
    if (m_input) m_input->setPlaceholderText(hint);
}

void ChatEventWidget::clearChoices()
{
    if (!m_choiceLayout) return;
    // 移除按钮（保留末尾 stretch）。
    while (m_choiceLayout->count() > 1) {
        auto *item = m_choiceLayout->takeAt(0);
        if (auto *w = item ? item->widget() : nullptr) w->deleteLater();
        delete item;
    }
    if (m_choiceBar) m_choiceBar->hide();
}

void ChatEventWidget::presentChoices(const QStringList &labels)
{
    clearChoices();
    if (labels.isEmpty()) return;
    for (int i = 0; i < labels.size(); ++i) {
        auto *btn = new QPushButton(labels.at(i), m_choiceBar);
        btn->setObjectName("ChoiceButton");
        connect(btn, &QPushButton::clicked, this, [this, i] { emit choiceSelected(i); });
        m_choiceLayout->insertWidget(m_choiceLayout->count() - 1, btn);  // 插在 stretch 前
    }
    if (m_choiceBar) m_choiceBar->show();
}

void ChatEventWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    auto *title = new QLabel(TR("title.chat"), this);
    title->setObjectName("PanelTitle");
    m_titleLabel = title;

    m_eventView = new QTextEdit(this);
    m_eventView->setObjectName("ParchmentView");
    m_eventView->setReadOnly(true);
    m_eventView->document()->setDefaultStyleSheet(
        "p { margin: 8px 0; line-height: 140%; color: #d0c8b4; }"
        ".time { color: #8a7e62; font-size: 11px; }"
        "b { color: #e8d5a4; font-weight: 600; }"
    );

    auto *inputLayout = new QHBoxLayout;
    inputLayout->setSpacing(8);

    m_channelCombo = new QComboBox(this);
    m_channelLabels.clear();
    m_channelLabels << TR("chat.channel.location")
                    << TR("chat.channel.world")
                    << TR("chat.channel.party");
    m_channelCombo->addItems(m_channelLabels);
    m_channelCombo->setEditable(false);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(TR("chat.placeholder"));

    m_sendButton = new QPushButton(TR("chat.send"), this);
    m_sendButton->setObjectName("PrimaryButton");

    inputLayout->addWidget(m_channelCombo);
    inputLayout->addWidget(m_input, 1);
    inputLayout->addWidget(m_sendButton);

    layout->addWidget(title);

    // 场景 Header（开局剧情/场景切换时由控制器设置）。
    m_sceneHeaderLabel = new QLabel(this);
    m_sceneHeaderLabel->setObjectName("SceneHeader");
    m_sceneHeaderLabel->setWordWrap(true);
    m_sceneHeaderLabel->hide();  // 默认隐藏，剧情开始时显示
    layout->addWidget(m_sceneHeaderLabel);

    layout->addWidget(m_eventView, 1);
    layout->addLayout(inputLayout);

    // 选项按钮栏（开局剧情用，默认隐藏）。
    m_choiceBar = new QWidget(this);
    m_choiceBar->setObjectName("ChoiceBar");
    m_choiceLayout = new QHBoxLayout(m_choiceBar);
    m_choiceLayout->setContentsMargins(0, 0, 0, 0);
    m_choiceLayout->setSpacing(8);
    m_choiceLayout->addStretch();
    m_choiceBar->hide();
    layout->addWidget(m_choiceBar);

    connect(m_sendButton, &QPushButton::clicked, this, &ChatEventWidget::onSendClicked);
    connect(m_input, &QLineEdit::returnPressed, this, &ChatEventWidget::onSendClicked);

    // 语言切换时刷新标题、placeholder、按钮、频道下拉。
    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) { retranslateUi(); });
}

void ChatEventWidget::seedMessages()
{
    const QString now = QTime::currentTime().toString(QStringLiteral("hh:mm"));
    m_eventView->append(QStringLiteral("<p><b>%1</b><br>%2</p>")
                            .arg(TR("chat.system"), TR("chat.seed.breakfast")));
    m_eventView->append(QStringLiteral("<p><b>%1</b><br>%2</p>")
                            .arg(TR("npc.mcgonagall"), TR("chat.seed.class")));
    m_eventView->append(QStringLiteral("<p><b>[Library]</b> <b>%1:</b><br>%2</p>")
                            .arg(TR("npc.hermione"), TR("chat.seed.library")));
    m_eventView->append(QStringLiteral("<p><span class='time'>08:55</span> %1</p>")
                            .arg(TR("chat.seed.enter_class").arg(now)));
    m_eventView->append(QStringLiteral("<p><span class='time'>09:03</span> %1</p>")
                            .arg(TR("chat.seed.lesson_start").arg(now)));
}

void ChatEventWidget::retranslateUi()
{
    if (m_titleLabel) m_titleLabel->setText(TR("title.chat"));
    if (m_input)      m_input->setPlaceholderText(TR("chat.placeholder"));
    if (m_sendButton) m_sendButton->setText(TR("chat.send"));
    if (m_channelCombo) {
        const int cur = m_channelCombo->currentIndex();
        m_channelLabels.clear();
        m_channelLabels << TR("chat.channel.location")
                        << TR("chat.channel.world")
                        << TR("chat.channel.party");
        m_channelCombo->clear();
        m_channelCombo->addItems(m_channelLabels);
        m_channelCombo->setCurrentIndex(qBound(0, cur, m_channelLabels.size() - 1));
    }
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
