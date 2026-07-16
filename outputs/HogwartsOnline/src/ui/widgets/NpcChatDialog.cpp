#include "ui/widgets/NpcChatDialog.h"
#include "core/DeepSeekClient.h"
#include "core/LanguageManager.h"

#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

// ====== NPC 头像绘制（简易圆形头像）======
class NpcAvatarWidget : public QWidget {
public:
    explicit NpcAvatarWidget(const QString &name, QWidget *parent = nullptr)
        : QWidget(parent), m_name(name) {
        setFixedSize(64, 64);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // 圆形背景
        p.setBrush(QColor("#4b3324"));
        p.setPen(QPen(QColor("#c9a45c"), 2));
        p.drawEllipse(2, 2, 60, 60);

        // 首字母
        QFont font("Georgia", 28, QFont::Bold);
        p.setFont(font);
        p.setPen(QColor("#f3d27a"));
        p.drawText(QRect(2, 2, 60, 60), Qt::AlignCenter,
                   m_name.isEmpty() ? "?" : m_name.mid(0, 1).toUpper());
    }
private:
    QString m_name;
};

NpcChatDialog::NpcChatDialog(DeepSeekClient *client,
                             const NpcPersonality &personality,
                             QWidget *parent)
    : QDialog(parent)
    , m_client(client)
    , m_personality(personality)
{
    setObjectName("NpcChatDialog");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(700, 580);
    setMinimumSize(540, 440);
    buildUi();
    retranslateUi();

    connect(m_client, &DeepSeekClient::replyReceived,
            this, &NpcChatDialog::onReplyReceived);
}

void NpcChatDialog::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(16, 14, 16, 14);
    rootLayout->setSpacing(10);

    // 顶部：头像 + 名字 + 学院
    auto *headerLayout = new QHBoxLayout;
    auto *avatar = new NpcAvatarWidget(m_personality.displayNameEn, this);
    headerLayout->addWidget(avatar);

    auto *infoLayout = new QVBoxLayout;
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("LoginTitle");
    QFont nameFont("Georgia", 20, QFont::Bold);
    m_titleLabel->setFont(nameFont);

    m_houseLabel = new QLabel(this);
    m_houseLabel->setObjectName("LoginSubtitle");
    QFont houseFont("Georgia", 12, QFont::StyleItalic);
    m_houseLabel->setFont(houseFont);

    infoLayout->addWidget(m_titleLabel);
    infoLayout->addWidget(m_houseLabel);
    headerLayout->addLayout(infoLayout, 1);

    m_apiKeyButton = new QPushButton(this);
    m_apiKeyButton->setObjectName("SecondaryButton");
    m_apiKeyButton->setMinimumWidth(100);
    headerLayout->addWidget(m_apiKeyButton);

    rootLayout->addLayout(headerLayout);

    // 聊天视图
    m_chatView = new QTextEdit(this);
    m_chatView->setObjectName("ParchmentView");
    m_chatView->setReadOnly(true);
    rootLayout->addWidget(m_chatView, 1);

    // 输入区
    auto *inputLayout = new QHBoxLayout;
    inputLayout->setSpacing(8);
    m_input = new QLineEdit(this);
    m_input->setMinimumHeight(40);
    m_sendButton = new QPushButton(this);
    m_sendButton->setObjectName("PrimaryButton");
    m_sendButton->setMinimumHeight(40);
    m_sendButton->setMinimumWidth(70);
    inputLayout->addWidget(m_input, 1);
    inputLayout->addWidget(m_sendButton);
    rootLayout->addLayout(inputLayout);

    connect(m_sendButton, &QPushButton::clicked, this, &NpcChatDialog::onSendClicked);
    connect(m_input, &QLineEdit::returnPressed, this, &NpcChatDialog::onSendClicked);
    connect(m_apiKeyButton, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        const QString key = QInputDialog::getText(this, "DeepSeek API Key",
            "Enter your DeepSeek API key:", QLineEdit::Password,
            m_client->apiKey(), &ok);
        if (ok && !key.isEmpty()) {
            m_client->setApiKey(key);
            appendBubble("System",
                "API Key configured. Try sending a message now!", false);
        }
    });

    // 初始问候
    QTimer::singleShot(400, this, [this]() {
        auto &lm = LanguageManager::instance();
        bool isCn = lm.currentLanguage() == Language::Chinese;
        appendBubble(m_personality.displayNameEn,
                     isCn ? m_personality.greetingCn : m_personality.greetingEn,
                     true);
    });
}

void NpcChatDialog::onSendClicked()
{
    if (m_waiting) return;
    const QString text = m_input->text().trimmed();
    if (text.isEmpty()) return;

    appendBubble("You", text, false);
    m_history.append({"user", text});
    m_input->clear();
    m_waiting = true;

    m_client->sendMessage(m_personality.npcId,
                          systemPrompt(), text, m_history);
}

void NpcChatDialog::onReplyReceived(const QString &npcId, const QString &reply, bool ok)
{
    if (npcId != m_personality.npcId) return;

    m_waiting = false;
    if (ok) {
        appendBubble(m_personality.displayNameEn, reply, true);
        m_history.append({"assistant", reply});
    } else {
        appendBubble("Error", reply, false);
    }
}

void NpcChatDialog::appendBubble(const QString &speaker, const QString &text, bool isNpc)
{
    const QString color = isNpc ? "#3b2416" : "#5a3818";
    const QString bg = isNpc ? "#e4d5b7" : "#c9a45c";

    m_chatView->append(QString(
        "<div style='margin:8px 0;padding:6px 0;'>"
        "<b style='color:%1;'>%2</b>"
        "<div style='padding:8px 12px;margin:4px 0;background:%3;border-radius:8px;color:#2a1e14;'>"
        "%4</div></div>"
    ).arg(color, speaker.toHtmlEscaped(), bg, text.toHtmlEscaped().replace("\n", "<br>")));

    // 自动滚动
    QScrollBar *sb = m_chatView->verticalScrollBar();
    sb->setValue(sb->maximum());
}

QString NpcChatDialog::systemPrompt() const
{
    auto &lm = LanguageManager::instance();
    return lm.currentLanguage() == Language::Chinese
        ? m_personality.systemPromptCn
        : m_personality.systemPromptEn;
}

void NpcChatDialog::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = lm.currentLanguage() == Language::Chinese;

    setWindowTitle(isCn ? QString("与 %1 对话").arg(m_personality.displayNameCn)
                        : QString("Chat with %1").arg(m_personality.displayNameEn));
    m_titleLabel->setText(isCn ? m_personality.displayNameCn : m_personality.displayNameEn);
    m_houseLabel->setText(isCn
        ? QString("%1 · %2").arg(m_personality.house, m_personality.role)
        : QString("%1 · %2").arg(m_personality.house, m_personality.role));
    m_sendButton->setText(isCn ? "发送" : "Send");
    m_apiKeyButton->setText(isCn ? "设置API" : "Set Key");
    m_input->setPlaceholderText(isCn ? "输入消息..." : "Type a message...");
}

void NpcChatDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QDialog::changeEvent(event);
}
