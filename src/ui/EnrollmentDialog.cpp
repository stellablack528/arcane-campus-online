#include "ui/EnrollmentDialog.h"

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

EnrollmentDialog::EnrollmentDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Hogwarts Enrollment - Diagon Alley");
    setMinimumSize(560, 480);
    buildUi();
    loadWandData();
}

void EnrollmentDialog::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(12);

    m_pages = new QStackedWidget(this);
    root->addWidget(m_pages);

    // ─── Page 1: Character Info ──────────────────────────
    auto *page1 = new QWidget;
    auto *form1 = new QFormLayout(page1);
    form1->setSpacing(10);

    m_usernameEdit = new QLineEdit(page1);
    m_usernameEdit->setPlaceholderText("e.g. harry_potter");
    m_nicknameEdit = new QLineEdit(page1);
    m_nicknameEdit->setPlaceholderText("e.g. Harry Potter");
    m_houseCombo = new QComboBox(page1);
    m_houseCombo->addItems({"Ravenclaw", "Gryffindor", "Hufflepuff", "Slytherin"});
    m_genderCombo = new QComboBox(page1);
    m_genderCombo->addItems({"Male", "Female", "Other"});
    m_hairColorEdit = new QLineEdit(page1);
    m_hairColorEdit->setPlaceholderText("e.g. Black");
    m_eyeColorEdit = new QLineEdit(page1);
    m_eyeColorEdit->setPlaceholderText("e.g. Green");
    m_bloodStatusCombo = new QComboBox(page1);
    m_bloodStatusCombo->addItem("Half-Blood", "halfblood");
    m_bloodStatusCombo->addItem("Pure-Blood", "pureblood");
    m_bloodStatusCombo->addItem("Muggle-Born", "muggleborn");

    form1->addRow("Username:", m_usernameEdit);
    form1->addRow("Nickname:", m_nicknameEdit);
    form1->addRow("House:", m_houseCombo);
    form1->addRow("Gender:", m_genderCombo);
    form1->addRow("Hair Color:", m_hairColorEdit);
    form1->addRow("Eye Color:", m_eyeColorEdit);
    form1->addRow("Blood Status:", m_bloodStatusCombo);

    auto *btns1 = new QHBoxLayout;
    auto *next1 = new QPushButton("Next: Diagon Alley \xe2\x86\x92", page1);
    next1->setObjectName("PrimaryButton");
    btns1->addStretch();
    btns1->addWidget(next1);
    auto *layout1 = new QVBoxLayout(page1);
    layout1->addLayout(form1);
    layout1->addStretch();
    layout1->addLayout(btns1);
    m_pages->addWidget(page1);

    connect(next1, &QPushButton::clicked, this, [this] { m_pages->setCurrentIndex(1); });

    // ─── Page 2: Wand Selection (Ollivanders) ───────────
    auto *page2 = new QWidget;
    auto *form2 = new QFormLayout(page2);
    form2->setSpacing(10);

    m_woodCombo = new QComboBox(page2);
    m_coreCombo = new QComboBox(page2);
    m_lengthSpin = new QDoubleSpinBox(page2);
    m_lengthSpin->setRange(9.0, 14.0);
    m_lengthSpin->setSingleStep(0.25);
    m_lengthSpin->setDecimals(2);
    m_lengthSpin->setValue(11.0);
    m_lengthSpin->setSuffix(" inches");
    m_flexibilityCombo = new QComboBox(page2);
    m_handleStyleCombo = new QComboBox(page2);
    m_finishCombo = new QComboBox(page2);
    m_colorToneCombo = new QComboBox(page2);
    m_engravingEdit = new QLineEdit(page2);
    m_engravingEdit->setMaxLength(24);
    m_engravingEdit->setPlaceholderText("Optional (max 24 chars)");

    form2->addRow("Wood:", m_woodCombo);
    form2->addRow("Core:", m_coreCombo);
    form2->addRow("Length:", m_lengthSpin);
    form2->addRow("Flexibility:", m_flexibilityCombo);
    form2->addRow("Handle Style:", m_handleStyleCombo);
    form2->addRow("Finish:", m_finishCombo);
    form2->addRow("Color Tone:", m_colorToneCombo);
    form2->addRow("Engraving:", m_engravingEdit);

    auto *btns2 = new QHBoxLayout;
    auto *back2 = new QPushButton("\xe2\x86\x90 Back", page2);
    auto *next2 = new QPushButton("Next: Pet \xe2\x86\x92", page2);
    next2->setObjectName("PrimaryButton");
    btns2->addWidget(back2);
    btns2->addStretch();
    btns2->addWidget(next2);
    auto *layout2 = new QVBoxLayout(page2);
    layout2->addLayout(form2);
    layout2->addStretch();
    layout2->addLayout(btns2);
    m_pages->addWidget(page2);

    connect(back2, &QPushButton::clicked, this, [this] { m_pages->setCurrentIndex(0); });
    connect(next2, &QPushButton::clicked, this, [this] { m_pages->setCurrentIndex(2); });

    // ─── Page 3: Pet Selection ──────────────────────────
    auto *page3 = new QWidget;
    auto *form3 = new QFormLayout(page3);
    form3->setSpacing(10);

    m_petCombo = new QComboBox(page3);
    m_petCombo->addItem("No pet (save your Galleons)", "none");
    m_petCombo->addItem("Owl (10 Galleons)", "owl");
    m_petCombo->addItem("Cat (8 Galleons)", "cat");
    m_petCombo->addItem("Toad (5 Galleons)", "toad");

    form3->addRow("Choose your companion:", m_petCombo);

    auto *btns3 = new QHBoxLayout;
    auto *back3 = new QPushButton("\xe2\x86\x90 Back", page3);
    auto *confirm = new QPushButton("Confirm Enrollment", page3);
    confirm->setObjectName("PrimaryButton");
    btns3->addWidget(back3);
    btns3->addStretch();
    btns3->addWidget(confirm);
    auto *layout3 = new QVBoxLayout(page3);
    layout3->addLayout(form3);
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
    const auto doc = QJsonDocument::fromJson(file.readAll());
    const auto root = doc.object();

    // Woods (38 options)
    const auto woods = root.value("official_wood_options").toArray();
    for (const auto &wood : woods) {
        const auto obj = wood.toObject();
        const QString id = obj.value("id").toString();
        const QString display = QStringLiteral("%1 (%2)")
                                    .arg(obj.value("name_en").toString())
                                    .arg(obj.value("name_zh").toString());
        m_woodCombo->addItem(display, id);
    }

    // Cores (3 Ollivander standards)
    const auto cores = root.value("official_ollivander_core_options").toArray();
    for (const auto &core : cores) {
        const auto obj = core.toObject();
        const QString id = obj.value("id").toString();
        const QString display = QStringLiteral("%1 (%2)")
                                    .arg(obj.value("name_en").toString())
                                    .arg(obj.value("name_zh").toString());
        m_coreCombo->addItem(display, id);
    }

    // Flexibility presets
    const auto flex = root.value("flexibility_presets_observed_on_official_pages").toArray();
    for (const auto &f : flex) {
        m_flexibilityCombo->addItem(f.toString());
    }

    // Cosmetic options
    const auto cosmetics = root.value("game_original_cosmetic_options").toObject();
    const auto handles = cosmetics.value("handle_style").toArray();
    for (const auto &h : handles) {
        m_handleStyleCombo->addItem(h.toString());
    }
    const auto finishes = cosmetics.value("finish").toArray();
    for (const auto &f : finishes) {
        m_finishCombo->addItem(f.toString());
    }
    const auto tones = cosmetics.value("color_tone").toArray();
    for (const auto &t : tones) {
        m_colorToneCombo->addItem(t.toString());
    }
}

arcane::application::dto::EnrollmentRequestDTO EnrollmentDialog::buildRequest() const
{
    using namespace arcane::application;
    dto::EnrollmentRequestDTO req;

    // Character info
    req.username = m_usernameEdit->text().trimmed().toStdString();
    req.nickname = m_nicknameEdit->text().trimmed().toStdString();
    req.house = m_houseCombo->currentText().toStdString();
    req.gender = m_genderCombo->currentText().toLower().toStdString();
    req.hairColor = m_hairColorEdit->text().trimmed().toStdString();
    req.eyeColor = m_eyeColorEdit->text().trimmed().toStdString();
    req.bloodStatus = m_bloodStatusCombo->currentData().toString().toStdString();

    // Wand selection
    req.wandWoodId = m_woodCombo->currentData().toString().toStdString();
    req.wandCoreId = m_coreCombo->currentData().toString().toStdString();
    req.wandLength = m_lengthSpin->value();
    req.wandFlexibility = m_flexibilityCombo->currentText().toStdString();
    req.wandHandleStyle = m_handleStyleCombo->currentText().toStdString();
    req.wandFinish = m_finishCombo->currentText().toStdString();
    req.wandColorTone = m_colorToneCombo->currentText().toStdString();
    req.wandEngraving = m_engravingEdit->text().trimmed().toStdString();

    // Pet
    req.petType = m_petCombo->currentData().toString().toStdString();

    return req;
}
