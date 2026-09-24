#include "ui/widgets/ChatEventWidget.h"

#include "ui/I18n.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QTextDocument>
#include <QTime>
#include <QVBoxLayout>

namespace {

QString escapedBody(QString text)
{
    text = text.toHtmlEscaped();
    return text.replace(QStringLiteral("\n"), QStringLiteral("<br>"));
}

bool equalsAny(const QString &value, std::initializer_list<const char *> candidates)
{
    for (const char *candidate : candidates) {
        if (value.compare(QString::fromLatin1(candidate), Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace

ChatEventWidget::ChatEventWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("ChatEventWidget");
    buildUi();
    seedMessages();
}

void ChatEventWidget::appendMessage(const QString &channel, const QString &speaker, const QString &text)
{
    appendEntry(classifyMessage(channel, speaker), channel, speaker, text);
}

void ChatEventWidget::appendRichMessage(const QString &channel, const QString &speaker, const QString &htmlBody)
{
    appendEntry(classifyMessage(channel, speaker), channel, speaker, htmlBody, true);
}

ChatEventWidget::MessageKind ChatEventWidget::classifyMessage(const QString &channel,
                                                              const QString &speaker) const
{
    const QString normalizedChannel = channel.trimmed();
    const QString normalizedSpeaker = speaker.trimmed();

    if (equalsAny(normalizedSpeaker, {"You", "Me", "Player"})
        || normalizedSpeaker == QStringLiteral("我")) {
        return MessageKind::PlayerDialogue;
    }
    if (equalsAny(normalizedSpeaker, {"Narrator", "Narrative"})
        || equalsAny(normalizedChannel, {"Prologue", "Story"})) {
        return MessageKind::Narration;
    }
    if (equalsAny(normalizedSpeaker, {"System", "Campus Notice", "House Points"})
        || normalizedSpeaker == QStringLiteral("系统")) {
        return MessageKind::SystemNotice;
    }
    if (equalsAny(normalizedSpeaker,
                  {"World", "Library", "Patrol", "Night Patrol", "Marauder's Map", "Reveal"})
        || equalsAny(normalizedChannel, {"Map", "House Cup"})) {
        return MessageKind::WorldEvent;
    }
    return MessageKind::NpcDialogue;
}

void ChatEventWidget::appendEntry(MessageKind kind,
                                  const QString &channel,
                                  const QString &speaker,
                                  const QString &body,
                                  bool bodyIsTrustedHtml)
{
    const QString time = QTime::currentTime().toString("hh:mm");
    const QString safeChannel = channel.toHtmlEscaped();
    const QString safeSpeaker = speaker.toHtmlEscaped();
    const QString safeBody = bodyIsTrustedHtml ? body : escapedBody(body);
    const QString context = safeChannel.isEmpty()
        ? time
        : QStringLiteral("%1 &nbsp;&middot;&nbsp; %2").arg(safeChannel, time);

    QString html;
    switch (kind) {
    case MessageKind::Narration:
        html = QStringLiteral(
            "<table width='96%' align='center' cellspacing='0' cellpadding='14' bgcolor='#29261d'>"
            "<tr><td style='border-left:3px solid #b89860;'>"
            "<span style='color:#b89860; font-size:10px; font-weight:700; letter-spacing:1px;'>"
            "&#10022;&nbsp; %1</span>"
            "<span style='color:#7f887d; font-size:10px;'>&nbsp;&nbsp;%2</span><br><br>"
            "<span style='color:#ded4bd; font-family:Georgia; font-size:15px;'>%3</span>"
            "</td></tr></table>")
            .arg(TR("chat.kind.narration"), time, safeBody);
        break;
    case MessageKind::NpcDialogue:
        html = QStringLiteral(
            "<table width='82%' align='left' cellspacing='0' cellpadding='12' bgcolor='#20271f'>"
            "<tr><td style='border-left:3px solid #667b69;'>"
            "<span style='color:#ead6a2; font-size:13px; font-weight:700;'>%1</span>"
            "<span style='color:#7f887d; font-size:10px;'>&nbsp;&middot;&nbsp; %2</span><br><br>"
            "<span style='color:#d6d9cf; font-size:14px;'>%3</span>"
            "</td></tr></table>")
            .arg(safeSpeaker, context, safeBody);
        break;
    case MessageKind::PlayerDialogue:
        html = QStringLiteral(
            "<table width='76%' align='right' cellspacing='0' cellpadding='12' bgcolor='#26342b'>"
            "<tr><td align='right' style='border-right:3px solid #b89860;'>"
            "<span style='color:#e8d5a4; font-size:12px; font-weight:700;'>%1</span>"
            "<span style='color:#88938a; font-size:10px;'>&nbsp;&middot;&nbsp; %2</span><br><br>"
            "<span style='color:#edf0e8; font-size:14px;'>%3</span>"
            "</td></tr></table>")
            .arg(TR("chat.you"), context, safeBody);
        break;
    case MessageKind::WorldEvent:
        html = QStringLiteral(
            "<table width='90%' align='center' cellspacing='0' cellpadding='10' bgcolor='#1d2927'>"
            "<tr><td align='center' style='border-top:1px solid #38544f; border-bottom:1px solid #38544f;'>"
            "<span style='color:#8db0a5; font-size:11px; font-weight:700;'>&#9671;&nbsp; %1</span>"
            "<span style='color:#71847d; font-size:10px;'>&nbsp;&middot;&nbsp; %2</span><br><br>"
            "<span style='color:#c7d5cf; font-size:13px;'>%3</span>"
            "</td></tr></table>")
            .arg(safeSpeaker.isEmpty() ? TR("chat.kind.world") : safeSpeaker,
                 context,
                 safeBody);
        break;
    case MessageKind::SystemNotice:
        html = QStringLiteral(
            "<table width='84%' align='center' cellspacing='0' cellpadding='9' bgcolor='#191d19'>"
            "<tr><td align='center' style='border:1px solid #333d35;'>"
            "<span style='color:#9a8b68; font-size:10px; font-weight:700;'>&#9670;&nbsp; %1</span>"
            "<span style='color:#777f76; font-size:10px;'>&nbsp;&nbsp;%2</span><br>"
            "<span style='color:#aeb6aa; font-size:12px;'>%3</span>"
            "</td></tr></table>")
            .arg(TR("chat.kind.notice"), time, safeBody);
        break;
    }

    m_eventView->append(html);
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
    layout->setContentsMargins(22, 20, 22, 18);
    layout->setSpacing(12);

    auto *title = new QLabel(TR("title.chat"), this);
    title->setObjectName("PanelTitle");
    m_titleLabel = title;

    m_eventView = new QTextEdit(this);
    m_eventView->setObjectName("ParchmentView");
    m_eventView->setReadOnly(true);
    m_eventView->document()->setDocumentMargin(18);
    m_eventView->document()->setDefaultStyleSheet(
        "p { margin: 7px 0; color: #d0c8b4; }"
        "table { margin-top: 7px; margin-bottom: 7px; }"
        "b { color: #efdca8; font-weight: 700; }"
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
    appendEntry(MessageKind::SystemNotice,
                QStringLiteral("System"),
                QStringLiteral("Campus Notice"),
                TR("chat.seed.breakfast"));
    appendEntry(MessageKind::NpcDialogue,
                TR("chat.channel.location"),
                TR("npc.mcgonagall"),
                TR("chat.seed.class"));
    appendEntry(MessageKind::NpcDialogue,
                TR("loc.library"),
                TR("npc.hermione"),
                TR("chat.seed.library"));
    appendEntry(MessageKind::WorldEvent,
                TR("chat.channel.location"),
                TR("chat.kind.world"),
                TR("chat.seed.enter_class"));
    appendEntry(MessageKind::WorldEvent,
                TR("chat.channel.location"),
                TR("npc.mcgonagall"),
                TR("chat.seed.lesson_start"));
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
