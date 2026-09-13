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
    setMinimumSize(560, 520);
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

    // --- Page 1: Character Info ---
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
    m_personalityCombo = new QComboBox(page1);
    m_personalityCombo->addItems({"Brave", "Wise", "Loyal", "Ambitious", "Creative", "Intuitive"});

    form1->addRow("Username:", m_usernameEdit);
    form1->addRow("Nickname:", m_nicknameEdit);
    form1->addRow("House:", m_houseCombo);
    form1->addRow("Gender:", m_genderCombo);
    form1->addRow("Hair Color:", m_hairColorEdit);
    form1->addRow("Eye Color:", m_eyeColorEdit);
    form1->addRow("Blood Status:", m_bloodStatusCombo);
    form1->addRow("Personality:", m_personalityCombo);

    auto *btns1 = new QHBoxLayout;
    auto *next1 = new QPushButton("Next: Ollivanders \xe2\x86\x92", page1);
    next1->setObjectName("PrimaryButton");
    btns1->addStretch();
    btns1->addWidget(next1);
    auto *layout1 = new QVBoxLayout(page1);
    layout1->addLayout(form1);
    layout1->addStretch();
    layout1->addLayout(btns1);
    m_pages->addWidget(page1);

    connect(next1, &QPushButton::clicked, this, [this] { m_pages->setCurrentIndex(1); });

    // --- Page 2: Wand Selection (Ollivanders) ---
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
    auto *next2 = new QPushButton("Next: Your Wand \xe2\x86\x92", page2);
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
    // Switching to Page 3 generates the wand lore description.
    connect(next2, &QPushButton::clicked, this, [this] {
        m_pages->setCurrentIndex(2);
        m_wandLoreLabel->setText(generateWandLore());
    });

    // --- Page 3: Wand Lore + Pet Selection ---
    auto *page3 = new QWidget;
    auto *layout3 = new QVBoxLayout(page3);
    layout3->setSpacing(12);

    auto *loreTitle = new QLabel("Your Wand Has Chosen You", page3);
    loreTitle->setObjectName("PanelTitle");
    m_wandLoreLabel = new QLabel(page3);
    m_wandLoreLabel->setWordWrap(true);
    m_wandLoreLabel->setObjectName("ValueLabel");
    m_wandLoreLabel->setMinimumHeight(120);

    auto *form3 = new QFormLayout;
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

    layout3->addWidget(loreTitle);
    layout3->addWidget(m_wandLoreLabel);
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

    // Woods (38 options) — cache traits for lore generation.
    const auto woods = root.value("official_wood_options").toArray();
    for (const auto &wood : woods) {
        const auto obj = wood.toObject();
        const QString id = obj.value("id").toString();
        const QString display = QStringLiteral("%1 (%2)")
                                    .arg(obj.value("name_en").toString())
                                    .arg(obj.value("name_zh").toString());
        m_woodCombo->addItem(display, id);
        m_woodTraits[id] = obj.value("traits").toString();
    }

    // Cores (3 Ollivander standards) — cache traits.
    const auto cores = root.value("official_ollivander_core_options").toArray();
    for (const auto &core : cores) {
        const auto obj = core.toObject();
        const QString id = obj.value("id").toString();
        const QString display = QStringLiteral("%1 (%2)")
                                    .arg(obj.value("name_en").toString())
                                    .arg(obj.value("name_zh").toString());
        m_coreCombo->addItem(display, id);
        m_coreTraits[id] = obj.value("traits").toString();
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

QString EnrollmentDialog::generateWandLore() const
{
    const QString woodId = m_woodCombo->currentData().toString();
    const QString coreId = m_coreCombo->currentData().toString();
    const QString woodName = m_woodCombo->currentText().section(" (", 0, 0);
    const QString coreName = m_coreCombo->currentText().section(" (", 0, 0);
    const QString woodTraits = m_woodTraits.value(woodId);
    const QString coreTraits = m_coreTraits.value(coreId);
    const double length = m_lengthSpin->value();
    const QString flex = m_flexibilityCombo->currentText();
    const QString personality = m_personalityCombo->currentText();

    // Length interpretation
    QString lengthDesc;
    if (length < 10.5)
        lengthDesc = QStringLiteral("shorter and more precise \xe2\x80\x94 suited to quick, focused spellwork");
    else if (length > 12.5)
        lengthDesc = QStringLiteral("longer and more powerful \xe2\x80\x94 built for dramatic, large-scale magic");
    else
        lengthDesc = QStringLiteral("balanced in length \xe2\x80\x94 versatile across many forms of magic");

    // Flexibility interpretation
    QString flexDesc;
    if (flex.contains("Unbending") || flex.contains("Rigid") || flex.contains("Solid"))
        flexDesc = QStringLiteral("Its rigidity speaks of a wielder who does not waver \xe2\x80\x94 "
                                  "your will matches the wand's own stubbornness.");
    else if (flex.contains("swishy") || flex.contains("springy") || flex.contains("bendy"))
        flexDesc = QStringLiteral("Its flexibility suggests a wielder who adapts and grows \xe2\x80\x94 "
                                  "the wand will learn and change with you.");
    else
        flexDesc = QStringLiteral("Its gentle suppleness hints at a balanced nature \xe2\x80\x94 "
                                  "neither rigid nor careless.");

    return QStringLiteral(
        "You are %1 by nature. The %2-inch %3 has chosen you \xe2\x80\x94 %4\n\n"
        "Paired with a %5 core, %6\n\n"
        "Its length is %7.\n"
        "%8\n\n"
        "Remember: the wand chooses the wizard. This one chose you."
    ).arg(personality.toLower())
     .arg(length, 0, 'f', 2)
     .arg(woodName)
     .arg(woodTraits.isEmpty() ? QStringLiteral("a wood of subtle power") : woodTraits)
     .arg(coreName)
     .arg(coreTraits.isEmpty() ? QStringLiteral("a rare and powerful combination") : coreTraits)
     .arg(lengthDesc)
     .arg(flexDesc);
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
    // Store the generated lore as the wand description.
    req.wandDescription = generateWandLore().toStdString();

    // Pet
    req.petType = m_petCombo->currentData().toString().toStdString();

    return req;
}
