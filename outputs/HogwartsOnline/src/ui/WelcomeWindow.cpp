#include "ui/WelcomeWindow.h"
#include "core/LanguageManager.h"

#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

// 自定义绘制霍格沃茨校徽
class HogwartsCrestWidget : public QWidget
{
public:
    explicit HogwartsCrestWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumSize(220, 220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const int w = width();
        const int h = height();
        const QPointF center(w / 2.0, h / 2.0);
        const qreal radius = qMin(w, h) * 0.42;

        // 外圈
        QPen ringPen(QColor("#c9a45c"), 4);
        p.setPen(ringPen);
        p.setBrush(QColor("#2a1b14"));
        p.drawEllipse(center, radius, radius);

        // 中心字母 H
        QFont hFont("Georgia", qMin(w, h) * 0.45, QFont::Bold);
        p.setFont(hFont);
        p.setPen(QColor("#f3d27a"));
        p.drawText(rect(), Qt::AlignCenter, "H");

        // 四个学院颜色小点（象征四学院）
        const qreal dotR = radius * 0.10;
        const QColor houses[4] = {
            QColor("#ae0001"), // Gryffindor
            QColor("#222f5b"), // Ravenclaw
            QColor("#ecb939"), // Hufflepuff
            QColor("#1a472a")  // Slytherin
        };
        for (int i = 0; i < 4; ++i) {
            const qreal angle = M_PI / 4.0 + i * (M_PI / 2.0);
            const QPointF pos(center.x() + radius * 0.72 * std::cos(angle),
                              center.y() - radius * 0.72 * std::sin(angle));
            p.setPen(Qt::NoPen);
            p.setBrush(houses[i]);
            p.drawEllipse(pos, dotR, dotR);
        }
    }
};

WelcomeWindow::WelcomeWindow(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("WelcomeWindow");
    setMinimumSize(900, 620);
    resize(1080, 720);
    setWindowTitle("Hogwarts Online");
    buildUi();
    retranslateUi();
}

void WelcomeWindow::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(60, 40, 60, 40);
    rootLayout->setSpacing(20);

    // 校徽
    auto *crest = new HogwartsCrestWidget(this);
    crest->setObjectName("WelcomeCrest");
    rootLayout->addWidget(crest, 0, Qt::AlignHCenter);

    // 标题
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("WelcomeTitle");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont("Georgia", 38, QFont::Bold);
    m_titleLabel->setFont(titleFont);
    rootLayout->addWidget(m_titleLabel);

    // 副标题
    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setObjectName("WelcomeSubtitle");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    QFont subFont("Georgia", 14, QFont::StyleItalic);
    m_subtitleLabel->setFont(subFont);
    rootLayout->addWidget(m_subtitleLabel);

    // 引言（邓布利多语录）
    m_quoteLabel = new QLabel(this);
    m_quoteLabel->setObjectName("WelcomeQuote");
    m_quoteLabel->setAlignment(Qt::AlignCenter);
    m_quoteLabel->setWordWrap(true);
    QFont quoteFont("Georgia", 12, QFont::StyleItalic);
    m_quoteLabel->setFont(quoteFont);
    m_quoteLabel->setMaximumWidth(680);
    rootLayout->addWidget(m_quoteLabel, 0, Qt::AlignHCenter);

    rootLayout->addStretch(1);

    // 语言选择
    auto *langRow = new QHBoxLayout;
    langRow->setSpacing(12);
    m_langLabel = new QLabel(this);
    m_langLabel->setObjectName("WelcomeLangLabel");
    QFont langLabelFont("Georgia", 13);
    m_langLabel->setFont(langLabelFont);
    m_langCombo = new QComboBox(this);
    m_langCombo->addItem(QStringLiteral("中文"), QVariant(1));
    m_langCombo->addItem(QStringLiteral("English"), QVariant(0));
    m_langCombo->setCurrentIndex(0); // 默认中文
    m_langCombo->setMinimumWidth(160);
    m_langCombo->setMinimumHeight(36);
    langRow->addStretch();
    langRow->addWidget(m_langLabel);
    langRow->addWidget(m_langCombo);
    langRow->addStretch();
    rootLayout->addLayout(langRow);

    // 开始按钮
    m_startButton = new QPushButton(this);
    m_startButton->setObjectName("PrimaryButton");
    QFont btnFont("Georgia", 16, QFont::Bold);
    m_startButton->setFont(btnFont);
    m_startButton->setMinimumSize(220, 56);
    m_startButton->setCursor(Qt::PointingHandCursor);
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(m_startButton);
    btnRow->addStretch();
    rootLayout->addLayout(btnRow);

    connect(m_startButton, &QPushButton::clicked, this, [this]() {
        const int idx = m_langCombo->currentData().toInt();
        LanguageManager::instance().setLanguage(static_cast<Language>(idx));
        emit startRequested();
    });
}

void WelcomeWindow::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    m_titleLabel->setText(lm.tr("Welcome to Hogwarts", "欢迎来到霍格沃茨"));
    m_subtitleLabel->setText(lm.tr("School of Witchcraft and Wizardry",
                                    "魔法与巫术学院"));
    m_quoteLabel->setText(lm.tr(
        "\u201CHappiness can be found, even in the darkest of times, "
        "if one only remembers to turn on the light.\u201D\n"
        " \u2014 Albus Dumbledore",
        "「即使在最黑暗的时刻，只要记得点亮灯火，就能找到幸福。」\n"
        " \u2014 阿不思·邓布利多"));
    m_langLabel->setText(lm.tr("Language: ", "语言："));
    m_startButton->setText(lm.tr("Begin the Journey", "开启魔法之旅"));
}

void WelcomeWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void WelcomeWindow::applyTheme()
{
    // 主题已在 QSS 中定义
}
