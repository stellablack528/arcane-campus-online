#include "ui/LoginWindow.h"
#include "core/LanguageManager.h"

#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QSizePolicy>
#include <QSlider>
#include <QStackedWidget>
#include <QVBoxLayout>

// ====== 魔杖预览组件（自定义绘制） ======
class WandPreviewWidget : public QWidget {
public:
    explicit WandPreviewWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumHeight(120);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    void setWand(const WandInfo &wand) {
        m_wand = wand;
        update();
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // 羊皮纸背景
        p.fillRect(rect(), QColor("#d8c39a"));

        // 杖木颜色映射
        QColor woodColor("#5b3a1e");
        const QString w = m_wand.wood.toLower();
        if (w.contains("holly")) woodColor = QColor("#8b2c1f");
        else if (w.contains("oak") || w.contains("english oak")) woodColor = QColor("#6b4423");
        else if (w.contains("willow")) woodColor = QColor("#a0825a");
        else if (w.contains("vine")) woodColor = QColor("#3e2814");
        else if (w.contains("elder")) woodColor = QColor("#2b1a0d");
        else if (w.contains("yew")) woodColor = QColor("#4a1e1a");
        else if (w.contains("hawthorn")) woodColor = QColor("#7a5230");
        else if (w.contains("cedar")) woodColor = QColor("#a06b3f");
        else if (w.contains("cherry")) woodColor = QColor("#8a3f2b");
        else if (w.contains("birch")) woodColor = QColor("#c9a876");
        else if (w.contains("maple")) woodColor = QColor("#b67a3d");
        else if (w.contains("ash")) woodColor = QColor("#caa777");
        else if (w.contains("elm")) woodColor = QColor("#7a5a3a");

        // 绘制魔杖（中心水平）
        const int h = height();
        const int w = width();
        const int wandLen = qMax(120, w - 80);
        const int wandH = 10;
        const int yMid = h / 2;

        // 杖身
        QRectF wandRect(40, yMid - wandH/2, wandLen, wandH);
        p.setBrush(woodColor);
        p.setPen(QPen(QColor("#3a2010"), 1));
        p.drawRoundedRect(wandRect, 5, 5);

        // 杖柄装饰
        p.setBrush(QColor("#f3d27a"));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(40 + wandLen - 4, yMid), 6, 6);

        // 杖芯光点（闪亮效果）
        p.setBrush(QColor("#fff4c2"));
        p.drawEllipse(QPointF(40 + wandLen * 0.25, yMid), 4, 4);
        p.drawEllipse(QPointF(40 + wandLen * 0.5, yMid), 3, 3);
        p.drawEllipse(QPointF(40 + wandLen * 0.75, yMid), 3, 3);
    }
private:
    WandInfo m_wand;
};

// ====== LoginWindow 实现 ======
LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("LoginWindow");
    setWindowTitle("Hogwarts Online - Sorting Ceremony");
    setMinimumSize(960, 660);
    resize(1080, 720);
    buildUi();
    retranslateUi();
    gotoStep(0);
}

void LoginWindow::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(50, 30, 50, 30);
    rootLayout->setSpacing(14);

    // 标题
    auto *title = new QLabel("Hogwarts Online", this);
    title->setObjectName("LoginTitle");
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont("Georgia", 28, QFont::Bold);
    title->setFont(titleFont);
    rootLayout->addWidget(title);

    // 步骤指示
    m_stepIndicator = new QLabel(this);
    m_stepIndicator->setObjectName("LoginStepIndicator");
    m_stepIndicator->setAlignment(Qt::AlignCenter);
    QFont stepFont("Georgia", 12);
    m_stepIndicator->setFont(stepFont);
    rootLayout->addWidget(m_stepIndicator);

    // StackedWidget 容纳多步骤
    m_stack = new QStackedWidget(this);
    m_stack->setObjectName("LoginStack");
    buildStepName();
    buildStepHouse();
    buildStepWand();
    rootLayout->addWidget(m_stack, 1);

    // 底部按钮
    auto *buttonRow = new QHBoxLayout;
    buttonRow->setSpacing(12);
    m_backButton = new QPushButton(this);
    m_backButton->setObjectName("SecondaryButton");
    m_backButton->setMinimumSize(120, 44);
    buttonRow->addWidget(m_backButton);
    buttonRow->addStretch();
    m_nextButton = new QPushButton(this);
    m_nextButton->setObjectName("PrimaryButton");
    m_nextButton->setMinimumSize(140, 44);
    m_nextButton->setDefault(true);
    m_nextButton->setCursor(Qt::PointingHandCursor);
    buttonRow->addWidget(m_nextButton);
    m_loginButton = new QPushButton(this);
    m_loginButton->setObjectName("PrimaryButton");
    m_loginButton->setMinimumSize(160, 44);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setVisible(false);
    buttonRow->addWidget(m_loginButton);
    rootLayout->addLayout(buttonRow);

    connect(m_nextButton, &QPushButton::clicked, this, &LoginWindow::onNextClicked);
    connect(m_backButton, &QPushButton::clicked, this, &LoginWindow::onBackClicked);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
}

void LoginWindow::buildStepName()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 30, 40, 30);
    layout->setSpacing(14);

    auto *prompt = new QLabel(page);
    prompt->setObjectName("StepPrompt");
    prompt->setAlignment(Qt::AlignCenter);
    prompt->setWordWrap(true);
    QFont promptFont("Georgia", 16);
    prompt->setFont(promptFont);
    prompt->setText("The Sorting Hat is ready...\nWhat is your name, young witch or wizard?");
    layout->addWidget(prompt);

    auto *nameLabel = new QLabel(page);
    nameLabel->setObjectName("FieldLabel");
    QFont fieldFont("Georgia", 12);
    nameLabel->setFont(fieldFont);
    layout->addWidget(nameLabel);

    m_nameEdit = new QLineEdit(page);
    m_nameEdit->setObjectName("LoginInput");
    m_nameEdit->setPlaceholderText("e.g. Hermione Granger");
    m_nameEdit->setText("New Student");
    m_nameEdit->setMinimumHeight(46);
    m_nameEdit->setClearButtonEnabled(true);
    QFont inputFont("Georgia", 13);
    m_nameEdit->setFont(inputFont);
    layout->addWidget(m_nameEdit);

    layout->addStretch();
    m_stack->addWidget(page);
}

void LoginWindow::buildStepHouse()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 30, 40, 30);
    layout->setSpacing(14);

    auto *prompt = new QLabel(page);
    prompt->setObjectName("StepPrompt");
    prompt->setAlignment(Qt::AlignCenter);
    prompt->setWordWrap(true);
    QFont promptFont("Georgia", 16);
    prompt->setFont(promptFont);
    prompt->setText("Place the Hat upon your head...\nWhich House calls to you?");
    layout->addWidget(prompt);

    auto *houseLabel = new QLabel(page);
    houseLabel->setObjectName("FieldLabel");
    QFont fieldFont("Georgia", 12);
    houseLabel->setFont(fieldFont);
    layout->addWidget(houseLabel);

    m_houseCombo = new QComboBox(page);
    m_houseCombo->addItem("🦅 Ravenclaw  ·  拉文克劳");
    m_houseCombo->addItem("🦁 Gryffindor  ·  格兰芬多");
    m_houseCombo->addItem("🦡 Hufflepuff  ·  赫奇帕奇");
    m_houseCombo->addItem("🐍 Slytherin  ·  斯莱特林");
    m_houseCombo->setMinimumHeight(50);
    QFont cbFont("Georgia", 14);
    m_houseCombo->setFont(cbFont);
    layout->addWidget(m_houseCombo);

    // 学院说明
    auto *houseDesc = new QLabel(page);
    houseDesc->setObjectName("HouseDesc");
    houseDesc->setAlignment(Qt::AlignCenter);
    houseDesc->setWordWrap(true);
    QFont descFont("Georgia", 11, QFont::StyleItalic);
    houseDesc->setFont(descFont);
    houseDesc->setText(
        "Ravenclaw — Wit, learning, wisdom  ·  智慧、学习与睿智\n"
        "Gryffindor — Courage, bravery, daring  ·  勇气、胆识与冒险\n"
        "Hufflepuff — Loyalty, fairness, patience  ·  忠诚、公正与耐心\n"
        "Slytherin — Ambition, cunning, resourcefulness  ·  野心、狡猾与机谋"
    );
    layout->addWidget(houseDesc);

    layout->addStretch();
    m_stack->addWidget(page);
}

void LoginWindow::buildStepWand()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 20, 30, 20);
    layout->setSpacing(12);

    auto *prompt = new QLabel(page);
    prompt->setObjectName("StepPrompt");
    prompt->setAlignment(Qt::AlignCenter);
    prompt->setWordWrap(true);
    QFont promptFont("Georgia", 16);
    prompt->setFont(promptFont);
    prompt->setText("Ollivanders: The wand chooses the wizard...");
    layout->addWidget(prompt);

    // 魔杖预览
    auto *preview = new WandPreviewWidget(page);
    preview->setObjectName("WandPreview");
    layout->addWidget(preview);

    // 木质
    auto *woodLabel = new QLabel("Wand Wood", page);
    woodLabel->setObjectName("FieldLabel");
    QFont fieldFont("Georgia", 12);
    woodLabel->setFont(fieldFont);
    layout->addWidget(woodLabel);

    m_woodCombo = new QComboBox(page);
    // 经典杖木（取自 Pottermore / 官方资料）
    m_woodCombo->addItems({
        "Holly", "Oak", "Willow", "Vine", "Elder",
        "Yew", "Hawthorn", "Cedar", "Cherry", "Birch",
        "Maple", "Ash", "Elm", "Hazel", "Rowan"
    });
    m_woodCombo->setMinimumHeight(40);
    QFont cbFont("Georgia", 12);
    m_woodCombo->setFont(cbFont);
    layout->addWidget(m_woodCombo);

    // 杖芯
    auto *coreLabel = new QLabel("Wand Core", page);
    coreLabel->setObjectName("FieldLabel");
    coreLabel->setFont(fieldFont);
    layout->addWidget(coreLabel);

    m_coreCombo = new QComboBox(page);
    m_coreCombo->addItems({
        "Unicorn Hair",
        "Dragon Heartstring",
        "Phoenix Feather"
    });
    m_coreCombo->setMinimumHeight(40);
    m_coreCombo->setFont(cbFont);
    layout->addWidget(m_coreCombo);

    // 长度
    auto *lengthLabel = new QLabel("Length", page);
    lengthLabel->setObjectName("FieldLabel");
    lengthLabel->setFont(fieldFont);
    layout->addWidget(lengthLabel);

    auto *lengthRow = new QHBoxLayout;
    m_lengthSlider = new QSlider(Qt::Horizontal, page);
    m_lengthSlider->setRange(9, 15);   // 9" - 15"
    m_lengthSlider->setValue(11);
    m_lengthSlider->setTickPosition(QSlider::TicksBelow);
    m_lengthSlider->setTickInterval(1);
    m_lengthValueLabel = new QLabel("11\"", page);
    m_lengthValueLabel->setObjectName("ValueLabel");
    QFont valFont("Georgia", 12, QFont::Bold);
    m_lengthValueLabel->setFont(valFont);
    m_lengthValueLabel->setMinimumWidth(60);
    lengthRow->addWidget(m_lengthSlider, 1);
    lengthRow->addWidget(m_lengthValueLabel);
    layout->addLayout(lengthRow);

    // 柔韧度（可选）
    auto *flexLabel = new QLabel("Flexibility", page);
    flexLabel->setObjectName("FieldLabel");
    flexLabel->setFont(fieldFont);
    layout->addWidget(flexLabel);

    m_flexCombo = new QComboBox(page);
    m_flexCombo->addItems({
        "Unyielding",
        "Slightly Springy",
        "Springy",
        "Quite Bendy",
        "Swishy",
        "Flexible",
        "Supple",
        "Whippy"
    });
    m_flexCombo->setMinimumHeight(40);
    m_flexCombo->setFont(cbFont);
    layout->addWidget(m_flexCombo);

    m_stack->addWidget(page);

    // 信号连接：更新预览
    connect(m_woodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoginWindow::onWandChanged);
    connect(m_coreCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoginWindow::onWandChanged);
    connect(m_lengthSlider, &QSlider::valueChanged, this, &LoginWindow::onWandChanged);
    connect(m_flexCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoginWindow::onWandChanged);

    onWandChanged();  // 初始预览
}

void LoginWindow::onWandChanged()
{
    if (!m_woodCombo) return;
    WandInfo wand;
    wand.wood = m_woodCombo->currentText();
    wand.core = m_coreCombo->currentText();
    wand.lengthInches = m_lengthSlider->value();
    wand.flexibility = m_flexCombo->currentText();

    m_lengthValueLabel->setText(QString::number(wand.lengthInches) + "\"");

    // 更新预览
    auto *stackLayout = qobject_cast<QVBoxLayout*>(m_stack->widget(2)->layout());
    if (stackLayout) {
        for (int i = 0; i < stackLayout->count(); ++i) {
            auto *w = stackLayout->itemAt(i)->widget();
            auto *preview = qobject_cast<WandPreviewWidget*>(w);
            if (preview) {
                preview->setWand(wand);
                break;
            }
        }
    }
}

void LoginWindow::gotoStep(int step)
{
    m_currentStep = step;
    m_stack->setCurrentIndex(step);

    m_backButton->setVisible(step > 0);
    m_nextButton->setVisible(step < 2);
    m_loginButton->setVisible(step == 2);

    // 更新步骤指示
    auto &lm = LanguageManager::instance();
    QString stepText;
    if (lm.currentLanguage() == Language::Chinese) {
        stepText = QString("第 %1 步 / 共 3 步").arg(step + 1);
    } else {
        stepText = QString("Step %1 of 3").arg(step + 1);
    }
    m_stepIndicator->setText(stepText);
}

void LoginWindow::onNextClicked()
{
    if (m_currentStep == 0) {
        // 检查姓名
        if (m_nameEdit->text().trimmed().isEmpty()) {
            m_nameEdit->setFocus();
            return;
        }
    }
    gotoStep(m_currentStep + 1);
}

void LoginWindow::onBackClicked()
{
    gotoStep(m_currentStep - 1);
}

void LoginWindow::onLoginClicked()
{
    QString studentName = m_nameEdit->text().trimmed();
    if (studentName.isEmpty()) studentName = "New Student";

    QString houseName = m_houseCombo->currentText();
    // 只取学院英文部分（去掉 emoji 和中文）
    if (houseName.contains("Ravenclaw")) houseName = "Ravenclaw";
    else if (houseName.contains("Gryffindor")) houseName = "Gryffindor";
    else if (houseName.contains("Hufflepuff")) houseName = "Hufflepuff";
    else if (houseName.contains("Slytherin")) houseName = "Slytherin";

    WandInfo wand;
    wand.wood = m_woodCombo->currentText();
    wand.core = m_coreCombo->currentText();
    wand.lengthInches = m_lengthSlider->value();
    wand.flexibility = m_flexCombo->currentText();

    emit loginRequested(studentName, houseName, wand);
}

void LoginWindow::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    // 步骤指示（在 gotoStep 里设置）
    m_backButton->setText(isCn ? "上一步" : "Back");
    m_nextButton->setText(isCn ? "下一步 →" : "Next →");
    m_loginButton->setText(isCn ? "踏入校园" : "Enter Campus");

    // 步骤页 1：姓名
    auto *pageName = m_stack->widget(0);
    auto *pageNameLayout = qobject_cast<QVBoxLayout*>(pageName->layout());
    if (pageNameLayout) {
        auto *prompt = qobject_cast<QLabel*>(pageNameLayout->itemAt(0)->widget());
        if (prompt) {
            prompt->setText(isCn
                ? "分院帽已经就绪...\n年轻的巫师，你叫什么名字？"
                : "The Sorting Hat is ready...\nWhat is your name, young witch or wizard?");
        }
        auto *nameLabel = qobject_cast<QLabel*>(pageNameLayout->itemAt(1)->widget());
        if (nameLabel) {
            nameLabel->setText(isCn ? "你的姓名" : "Your Name");
        }
        m_nameEdit->setPlaceholderText(isCn ? "例如：赫敏·格兰杰" : "e.g. Hermione Granger");
    }

    // 步骤页 2：学院
    auto *pageHouse = m_stack->widget(1);
    auto *pageHouseLayout = qobject_cast<QVBoxLayout*>(pageHouse->layout());
    if (pageHouseLayout) {
        auto *prompt = qobject_cast<QLabel*>(pageHouseLayout->itemAt(0)->widget());
        if (prompt) {
            prompt->setText(isCn
                ? "请把分院帽戴在头上...\n哪个学院在呼唤你？"
                : "Place the Hat upon your head...\nWhich House calls to you?");
        }
        auto *houseLabel = qobject_cast<QLabel*>(pageHouseLayout->itemAt(1)->widget());
        if (houseLabel) {
            houseLabel->setText(isCn ? "选择学院" : "Choose Your House");
        }
        auto *houseDesc = qobject_cast<QLabel*>(pageHouseLayout->itemAt(3)->widget());
        if (houseDesc) {
            houseDesc->setText(isCn
                ? "🦅 拉文克劳 — 智慧、学习与睿智\n"
                  "🦁 格兰芬多 — 勇气、胆识与冒险\n"
                  "🦡 赫奇帕奇 — 忠诚、公正与耐心\n"
                  "🐍 斯莱特林 — 野心、狡猾与机谋"
                : "Ravenclaw — Wit, learning, wisdom\n"
                  "Gryffindor — Courage, bravery, daring\n"
                  "Hufflepuff — Loyalty, fairness, patience\n"
                  "Slytherin — Ambition, cunning, resourcefulness");
        }
    }

    // 步骤页 3：魔杖
    auto *pageWand = m_stack->widget(2);
    auto *pageWandLayout = qobject_cast<QVBoxLayout*>(pageWand->layout());
    if (pageWandLayout) {
        auto *prompt = qobject_cast<QLabel*>(pageWandLayout->itemAt(0)->widget());
        if (prompt) {
            prompt->setText(isCn
                ? "奥利凡德：魔杖选择巫师..."
                : "Ollivanders: The wand chooses the wizard...");
        }
        // 索引顺序：0=prompt, 1=preview, 2=woodLabel, 3=woodCombo,
        //           4=coreLabel, 5=coreCombo, 6=lengthLabel, 7=lengthRow, 8=flexLabel, 9=flexCombo
        auto setField = [&](int idx, const QString &en, const QString &cn) {
            auto *lbl = qobject_cast<QLabel*>(pageWandLayout->itemAt(idx)->widget());
            if (lbl) lbl->setText(isCn ? cn : en);
        };
        setField(2, "Wand Wood", "杖木");
        setField(4, "Wand Core", "杖芯");
        setField(6, "Length", "长度");
        setField(8, "Flexibility", "柔韧度");
    }

    // 更新步骤指示
    if (m_stepIndicator) {
        if (isCn)
            m_stepIndicator->setText(QString("第 %1 步 / 共 3 步").arg(m_currentStep + 1));
        else
            m_stepIndicator->setText(QString("Step %1 of 3").arg(m_currentStep + 1));
    }
}

void LoginWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}
