#include "ui/EnrollmentDialog.h"

#include "ui/I18n.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {

bool isChinese()
{
    return arcane::ui::I18n::instance().language() == arcane::ui::I18n::Lang::Chinese;
}

QString local(const char* chinese, const char* english)
{
    return isChinese() ? QString::fromUtf8(chinese) : QString::fromUtf8(english);
}

} // namespace

EnrollmentDialog::EnrollmentDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(local("霍格沃茨新生登记 · 对角巷", "Hogwarts Enrollment · Diagon Alley"));
    setMinimumSize(620, 580);
    buildUi();
    loadWandData();
}

void EnrollmentDialog::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(14);

    auto *eyebrow = new QLabel(local("霍格沃茨魔法学校 · 新生档案", "HOGWARTS SCHOOL · FIRST-YEAR RECORD"), this);
    eyebrow->setObjectName("SectionCaption");
    auto *heading = new QLabel(local("在进入对角巷之前，留下你的名字与魔杖线索。",
                                     "Before entering Diagon Alley, leave your name and wand clues."), this);
    heading->setObjectName("PanelTitle");
    heading->setWordWrap(true);
    root->addWidget(eyebrow);
    root->addWidget(heading);

    m_pages = new QStackedWidget(this);
    root->addWidget(m_pages, 1);

    // Page 1: identity
    auto *page1 = new QWidget;
    auto *layout1 = new QVBoxLayout(page1);
    auto *form1 = new QFormLayout;
    form1->setSpacing(11);

    m_usernameEdit = new QLineEdit(page1);
    m_usernameEdit->setPlaceholderText(local("例如 luna_lovegood", "e.g. luna_lovegood"));
    m_nicknameEdit = new QLineEdit(page1);
    m_nicknameEdit->setPlaceholderText(local("角色在故事中的名字", "Your character's story name"));
    m_houseCombo = new QComboBox(page1);
    m_houseCombo->addItems({"Ravenclaw", "Gryffindor", "Hufflepuff", "Slytherin"});
    m_genderCombo = new QComboBox(page1);
    m_genderCombo->addItem(local("男", "Male"), "male");
    m_genderCombo->addItem(local("女", "Female"), "female");
    m_genderCombo->addItem(local("其他", "Other"), "other");
    m_hairColorEdit = new QLineEdit(page1);
    m_hairColorEdit->setPlaceholderText(local("例如 黑色", "e.g. Black"));
    m_eyeColorEdit = new QLineEdit(page1);
    m_eyeColorEdit->setPlaceholderText(local("例如 绿色", "e.g. Green"));
    m_bloodStatusCombo = new QComboBox(page1);
    m_bloodStatusCombo->addItem(local("混血", "Half-Blood"), "halfblood");
    m_bloodStatusCombo->addItem(local("纯血", "Pure-Blood"), "pureblood");
    m_bloodStatusCombo->addItem(local("麻瓜出身", "Muggle-Born"), "muggleborn");

    form1->addRow(local("用户名", "Username"), m_usernameEdit);
    form1->addRow(local("角色姓名", "Character name"), m_nicknameEdit);
    form1->addRow(local("学院", "House"), m_houseCombo);
    form1->addRow(local("性别", "Gender"), m_genderCombo);
    form1->addRow(local("发色", "Hair color"), m_hairColorEdit);
    form1->addRow(local("瞳色", "Eye color"), m_eyeColorEdit);
    form1->addRow(local("血统背景", "Blood status"), m_bloodStatusCombo);

    auto *btns1 = new QHBoxLayout;
    auto *next1 = new QPushButton(local("下一步 · 魔杖登记  →", "Next · Wand Order  →"), page1);
    next1->setObjectName("PrimaryButton");
    btns1->addStretch();
    btns1->addWidget(next1);
    layout1->addLayout(form1);
    layout1->addStretch();
    layout1->addLayout(btns1);
    m_pages->addWidget(page1);
    connect(next1, &QPushButton::clicked, this, [this] { m_pages->setCurrentIndex(1); });

    // Page 2: wand order. This records physical clues only—the story reveal is later.
    auto *page2 = new QWidget;
    auto *layout2 = new QVBoxLayout(page2);
    auto *explanation = new QLabel(local(
        "登记你偏好的魔杖参数。奥利凡德先生会提前准备相应的长匣，但只有当你亲手握住它时，选择才会真正完成。",
        "Record your preferred wand specifications. Mr. Ollivander will prepare the matching box, but the choice is complete only when you take it in hand."), page2);
    explanation->setWordWrap(true);
    explanation->setObjectName("ValueLabel");

    auto *form2 = new QFormLayout;
    form2->setSpacing(10);
    m_woodCombo = new QComboBox(page2);
    m_coreCombo = new QComboBox(page2);
    m_lengthSpin = new QDoubleSpinBox(page2);
    m_lengthSpin->setRange(9.0, 14.0);
    m_lengthSpin->setSingleStep(0.25);
    m_lengthSpin->setDecimals(2);
    m_lengthSpin->setValue(11.0);
    m_lengthSpin->setSuffix(local(" 英寸", " inches"));
    m_flexibilityCombo = new QComboBox(page2);
    m_handleStyleCombo = new QComboBox(page2);
    m_finishCombo = new QComboBox(page2);
    m_colorToneCombo = new QComboBox(page2);
    m_engravingEdit = new QLineEdit(page2);
    m_engravingEdit->setMaxLength(24);
    m_engravingEdit->setPlaceholderText(local("可选，最多 24 个字符", "Optional, up to 24 characters"));

    form2->addRow(local("木材", "Wood"), m_woodCombo);
    form2->addRow(local("杖芯", "Core"), m_coreCombo);
    form2->addRow(local("长度", "Length"), m_lengthSpin);
    form2->addRow(local("柔韧度", "Flexibility"), m_flexibilityCombo);
    form2->addRow(local("握柄样式", "Handle style"), m_handleStyleCombo);
    form2->addRow(local("表面处理", "Finish"), m_finishCombo);
    form2->addRow(local("色调", "Color tone"), m_colorToneCombo);
    form2->addRow(local("刻字", "Engraving"), m_engravingEdit);

    auto *btns2 = new QHBoxLayout;
    auto *back2 = new QPushButton(local("←  返回", "←  Back"), page2);
    auto *next2 = new QPushButton(local("封存登记单  →", "Seal the Order  →"), page2);
    next2->setObjectName("PrimaryButton");
    btns2->addWidget(back2);
    btns2->addStretch();
    btns2->addWidget(next2);
    layout2->addWidget(explanation);
    layout2->addLayout(form2);
    layout2->addStretch();
    layout2->addLayout(btns2);
    m_pages->addWidget(page2);
    connect(back2, &QPushButton::clicked, this, [this] { m_pages->setCurrentIndex(0); });
    connect(next2, &QPushButton::clicked, this, [this] {
        m_wandSummaryLabel->setText(buildWandSummary());
        m_pages->setCurrentIndex(2);
    });

    // Page 3: confirmation. No lore or pet choice here; both belong in the world.
    auto *page3 = new QWidget;
    auto *layout3 = new QVBoxLayout(page3);
    layout3->setSpacing(14);
    auto *summaryTitle = new QLabel(local("登记单已封存", "Your Order Is Sealed"), page3);
    summaryTitle->setObjectName("PanelTitle");
    m_wandSummaryLabel = new QLabel(page3);
    m_wandSummaryLabel->setWordWrap(true);
    m_wandSummaryLabel->setObjectName("ValueLabel");
    m_wandSummaryLabel->setMinimumHeight(150);
    auto *storyHint = new QLabel(local(
        "你不会在这里直接得到魔杖或宠物。进入世界后，你将站在对角巷；第一件事，是亲自走进奥利凡德魔杖店，让那根魔杖作出选择。",
        "You will not receive a wand or pet in this form. The story opens in Diagon Alley, where your first task is to enter Ollivanders and let the wand make its choice."), page3);
    storyHint->setWordWrap(true);

    auto *btns3 = new QHBoxLayout;
    auto *back3 = new QPushButton(local("←  修改魔杖", "←  Edit Wand"), page3);
    auto *confirm = new QPushButton(local("进入对角巷", "Enter Diagon Alley"), page3);
    confirm->setObjectName("PrimaryButton");
    btns3->addWidget(back3);
    btns3->addStretch();
    btns3->addWidget(confirm);
    layout3->addWidget(summaryTitle);
    layout3->addWidget(m_wandSummaryLabel);
    layout3->addWidget(storyHint);
    layout3->addStretch();
    layout3->addLayout(btns3);
    m_pages->addWidget(page3);
    connect(back3, &QPushButton::clicked, this, [this] { m_pages->setCurrentIndex(1); });
    connect(confirm, &QPushButton::clicked, this, &QDialog::accept);
}

void EnrollmentDialog::loadWandData()
{
    QFile file(":/data/wand_customization.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    const auto root = QJsonDocument::fromJson(file.readAll()).object();

    for (const auto &wood : root.value("official_wood_options").toArray()) {
        const auto object = wood.toObject();
        const QString displayName = isChinese()
            ? object.value("name_zh").toString()
            : object.value("name_en").toString();
        m_woodCombo->addItem(displayName, object.value("id").toString());
    }
    for (const auto &core : root.value("official_ollivander_core_options").toArray()) {
        const auto object = core.toObject();
        const QString displayName = isChinese()
            ? object.value("name_zh").toString()
            : object.value("name_en").toString();
        m_coreCombo->addItem(displayName, object.value("id").toString());
    }
    for (const auto &value : root.value("flexibility_presets_observed_on_official_pages").toArray()) {
        m_flexibilityCombo->addItem(value.toString());
    }

    const auto cosmetics = root.value("game_original_cosmetic_options").toObject();
    for (const auto &value : cosmetics.value("handle_style").toArray()) {
        m_handleStyleCombo->addItem(value.toString());
    }
    for (const auto &value : cosmetics.value("finish").toArray()) {
        m_finishCombo->addItem(value.toString());
    }
    for (const auto &value : cosmetics.value("color_tone").toArray()) {
        m_colorToneCombo->addItem(value.toString());
    }
}

QString EnrollmentDialog::buildWandSummary() const
{
    if (isChinese()) {
        return QStringLiteral(
            "木材：%1\n杖芯：%2\n长度：%3 英寸\n柔韧度：%4\n握柄与表面：%5 · %6 · %7%8")
            .arg(m_woodCombo->currentText(),
                 m_coreCombo->currentText(),
                 QString::number(m_lengthSpin->value(), 'f', 2),
                 m_flexibilityCombo->currentText(),
                 m_handleStyleCombo->currentText(),
                 m_finishCombo->currentText(),
                 m_colorToneCombo->currentText(),
                 m_engravingEdit->text().trimmed().isEmpty()
                     ? QString()
                     : QStringLiteral("\n刻字：%1").arg(m_engravingEdit->text().trimmed()));
    }
    return QStringLiteral(
        "Wood: %1\nCore: %2\nLength: %3 inches\nFlexibility: %4\nHandle and finish: %5 · %6 · %7%8")
        .arg(m_woodCombo->currentText(),
             m_coreCombo->currentText(),
             QString::number(m_lengthSpin->value(), 'f', 2),
             m_flexibilityCombo->currentText(),
             m_handleStyleCombo->currentText(),
             m_finishCombo->currentText(),
             m_colorToneCombo->currentText(),
             m_engravingEdit->text().trimmed().isEmpty()
                 ? QString()
                 : QStringLiteral("\nEngraving: %1").arg(m_engravingEdit->text().trimmed()));
}

arcane::application::dto::EnrollmentRequestDTO EnrollmentDialog::buildRequest() const
{
    arcane::application::dto::EnrollmentRequestDTO request;
    request.username = m_usernameEdit->text().trimmed().toStdString();
    request.nickname = m_nicknameEdit->text().trimmed().toStdString();
    request.house = m_houseCombo->currentText().toStdString();
    request.gender = m_genderCombo->currentData().toString().toStdString();
    request.hairColor = m_hairColorEdit->text().trimmed().toStdString();
    request.eyeColor = m_eyeColorEdit->text().trimmed().toStdString();
    request.bloodStatus = m_bloodStatusCombo->currentData().toString().toStdString();

    request.wandWoodId = m_woodCombo->currentData().toString().toStdString();
    request.wandCoreId = m_coreCombo->currentData().toString().toStdString();
    request.wandWoodName = m_woodCombo->currentText().toStdString();
    request.wandCoreName = m_coreCombo->currentText().toStdString();
    request.wandLength = m_lengthSpin->value();
    request.wandFlexibility = m_flexibilityCombo->currentText().toStdString();
    request.wandHandleStyle = m_handleStyleCombo->currentText().toStdString();
    request.wandFinish = m_finishCombo->currentText().toStdString();
    request.wandColorTone = m_colorToneCombo->currentText().toStdString();
    request.wandEngraving = m_engravingEdit->text().trimmed().toStdString();
    request.wandDescription.clear();
    request.petType = "none";
    return request;
}
