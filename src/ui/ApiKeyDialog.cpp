#include "ui/ApiKeyDialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

namespace {
constexpr const char* kSettingsKey = "ai/deepseek_api_key";
}

ApiKeyDialog::ApiKeyDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName("ApiKeyDialog");
    setWindowTitle("AI Settings - DeepSeek API Key");
    setMinimumWidth(480);
    buildUi();
}

void ApiKeyDialog::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto *title = new QLabel("DeepSeek API Key", this);
    title->setObjectName("PanelTitle");

    auto *hint = new QLabel(
        "NPC chat replies are generated with the DeepSeek API.\n"
        "Paste your own API key below — it is stored only on this computer\n"
        "(local QSettings) and is never bundled with the source code.", this);
    hint->setWordWrap(true);

    m_keyEdit = new QLineEdit(this);
    m_keyEdit->setEchoMode(QLineEdit::Password);
    m_keyEdit->setPlaceholderText("sk-...");

    QSettings settings(QStringLiteral("FanProject"), QStringLiteral("Hogwarts Online"));
    m_keyEdit->setText(settings.value(QLatin1String(kSettingsKey)).toString());

    auto *buttons = new QHBoxLayout;
    auto *saveButton = new QPushButton("Save", this);
    saveButton->setObjectName("PrimaryButton");
    auto *clearButton = new QPushButton("Clear Key", this);
    buttons->addWidget(saveButton);
    buttons->addWidget(clearButton);
    buttons->addStretch();

    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addWidget(m_keyEdit);
    layout->addLayout(buttons);

    connect(saveButton, &QPushButton::clicked, this, &ApiKeyDialog::onSaveClicked);
    connect(clearButton, &QPushButton::clicked, this, [this] {
        m_keyEdit->clear();
        onSaveClicked();
    });
    connect(m_keyEdit, &QLineEdit::returnPressed, this, &ApiKeyDialog::onSaveClicked);
}

void ApiKeyDialog::onSaveClicked()
{
    QSettings settings(QStringLiteral("FanProject"), QStringLiteral("Hogwarts Online"));
    settings.setValue(QLatin1String(kSettingsKey), m_keyEdit->text().trimmed());
    accept();
}
