#include "ui/LoginWindow.h"

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
    setWindowTitle("Hogwarts Online - Login");
    setMinimumSize(860, 560);
    resize(960, 620);
    buildUi();
}

void LoginWindow::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(64, 48, 64, 48);
    rootLayout->setSpacing(24);

    auto *title = new QLabel("Hogwarts Online", this);
    title->setObjectName("LoginTitle");
    title->setAlignment(Qt::AlignCenter);

    auto *subtitle = new QLabel("Autumn Term - Week Three - Student Entrance", this);
    subtitle->setObjectName("LoginSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    auto *panel = new QFrame(this);
    panel->setObjectName("LoginPanel");
    panel->setMinimumWidth(620);
    panel->setMaximumWidth(760);
    panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(44, 36, 44, 36);
    panelLayout->setSpacing(18);

    auto *nameLabel = new QLabel("Student Name", panel);
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

    auto *houseLabel = new QLabel("House", panel);
    m_houseCombo = new QComboBox(panel);
    m_houseCombo->addItems({"Ravenclaw", "Gryffindor", "Hufflepuff", "Slytherin"});
    m_houseCombo->setMinimumHeight(46);

    m_loginButton = new QPushButton("Enter Campus", panel);
    m_loginButton->setObjectName("PrimaryButton");
    m_loginButton->setMinimumHeight(48);
    m_loginButton->setDefault(true);

    panelLayout->addWidget(nameLabel);
    panelLayout->addWidget(m_nameEdit);
    panelLayout->addWidget(houseLabel);
    panelLayout->addWidget(m_houseCombo);
    panelLayout->addSpacing(8);
    panelLayout->addWidget(m_loginButton);

    auto *hint = new QLabel("Today: Breakfast is open in the Great Hall. First class begins at 09:00.", this);
    hint->setObjectName("LoginHint");
    hint->setAlignment(Qt::AlignCenter);

    rootLayout->addStretch();
    rootLayout->addWidget(title);
    rootLayout->addWidget(subtitle);
    rootLayout->addWidget(panel, 0, Qt::AlignHCenter);
    rootLayout->addWidget(hint);
    rootLayout->addStretch();

    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_nameEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
}

void LoginWindow::onLoginClicked()
{
    const QString studentName = m_nameEdit->text().trimmed().isEmpty()
        ? QStringLiteral("New Student")
        : m_nameEdit->text().trimmed();
    emit loginRequested(studentName, m_houseCombo->currentText());
}
