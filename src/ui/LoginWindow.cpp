#include "ui/LoginWindow.h"

#include "ui/I18n.hpp"

#include <QComboBox>
#include <QFrame>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("LoginWindow");
    setWindowTitle(QStringLiteral("Hogwarts Online - Login"));
    setMinimumSize(860, 560);
    resize(960, 620);
    buildUi();

    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) { retranslateUi(); });
}

void LoginWindow::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(64, 48, 64, 48);
    rootLayout->setSpacing(24);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("LoginTitle");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setObjectName("LoginSubtitle");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    auto *panel = new QFrame(this);
    panel->setObjectName("LoginPanel");
    panel->setMinimumWidth(620);
    panel->setMaximumWidth(760);
    panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(44, 36, 44, 36);
    panelLayout->setSpacing(18);

    m_nameLabel = new QLabel(panel);
    m_nameEdit = new QLineEdit(panel);
    m_nameEdit->setPlaceholderText("Julie Dumbledore");
    m_nameEdit->setText("New Student");
    m_nameEdit->setMinimumWidth(520);
    m_nameEdit->setMinimumHeight(46);
    m_nameEdit->setClearButtonEnabled(true);
    m_nameEdit->setFocus();

    QFont inputFont = m_nameEdit->font();
    inputFont.setPointSize(13);
    m_nameEdit->setFont(inputFont);

    m_houseLabel = new QLabel(panel);
    m_houseCombo = new QComboBox(panel);
    m_houseCombo->setMinimumHeight(46);

    m_loginButton = new QPushButton(panel);
    m_loginButton->setObjectName("PrimaryButton");
    m_loginButton->setMinimumHeight(48);
    m_loginButton->setDefault(true);

    panelLayout->addWidget(m_nameLabel);
    panelLayout->addWidget(m_nameEdit);
    panelLayout->addWidget(m_houseLabel);
    panelLayout->addWidget(m_houseCombo);
    panelLayout->addSpacing(8);
    panelLayout->addWidget(m_loginButton);
    panelLayout->addSpacing(4);
    m_enrollButton = new QPushButton(panel);
    m_enrollButton->setObjectName("ApiKeyDialog");
    panelLayout->addWidget(m_enrollButton);

    m_hintLabel = new QLabel(this);
    m_hintLabel->setObjectName("LoginHint");
    m_hintLabel->setAlignment(Qt::AlignCenter);

    rootLayout->addStretch();
    rootLayout->addWidget(m_titleLabel);
    rootLayout->addWidget(m_subtitleLabel);
    rootLayout->addWidget(panel, 0, Qt::AlignHCenter);
    rootLayout->addWidget(m_hintLabel);
    rootLayout->addStretch();

    retranslateUi();

    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_nameEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    connect(m_enrollButton, &QPushButton::clicked, this, &LoginWindow::newStudentRequested);
}

void LoginWindow::retranslateUi()
{
    if (m_titleLabel)    m_titleLabel->setText(TR("login.title"));
    if (m_subtitleLabel) m_subtitleLabel->setText(TR("login.subtitle"));
    if (m_nameLabel)     m_nameLabel->setText(TR("login.studentname"));
    if (m_houseLabel)    m_houseLabel->setText(TR("login.house"));
    if (m_loginButton)   m_loginButton->setText(TR("login.enter"));
    if (m_enrollButton)  m_enrollButton->setText(TR("login.enroll"));
    if (m_hintLabel)     m_hintLabel->setText(TR("login.hint"));

    if (m_houseCombo) {
        const int cur = m_houseCombo->currentIndex();
        m_houseCombo->clear();
        m_houseCombo->addItems({
            TR("house.ravenclaw"),
            TR("house.gryffindor"),
            TR("house.hufflepuff"),
            TR("house.slytherin"),
        });
        if (cur >= 0 && cur < m_houseCombo->count()) m_houseCombo->setCurrentIndex(cur);
    }
}

void LoginWindow::onLoginClicked()
{
    const QString studentName = m_nameEdit->text().trimmed().isEmpty()
        ? QStringLiteral("New Student")
        : m_nameEdit->text().trimmed();
    emit loginRequested(studentName, m_houseCombo->currentText());
}
