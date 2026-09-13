#include "ui/widgets/CharacterStatusWidget.h"

#include "ui/I18n.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

CharacterStatusWidget::CharacterStatusWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("CharacterStatusWidget");
    buildUi();
    retranslateUi();

    connect(&arcane::ui::I18n::instance(), &arcane::ui::I18n::languageChanged,
            this, [this](arcane::ui::I18n::Lang) { retranslateUi(); });
}

void CharacterStatusWidget::setIdentity(const QString &studentName, const QString &houseName)
{
    m_nameValue->setText(studentName);
    m_houseValue->setText(houseName);
}

void CharacterStatusWidget::setCampusState(const QString &location, const QString &state)
{
    m_locationValue->setText(location);
    m_stateValue->setText(state);
}

void CharacterStatusWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("PanelTitle");

    auto *form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignTop);
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(12);

    m_nameLabel     = new QLabel(this);
    m_houseLabel    = new QLabel(this);
    m_yearLabel     = new QLabel(this);
    m_locationLabel = new QLabel(this);
    m_galleonLabel  = new QLabel(this);
    m_pointsLabel   = new QLabel(this);
    m_stateLabel    = new QLabel(this);
    m_courseLabel   = new QLabel(this);

    m_nameValue     = createValue(QStringLiteral("New Student"));
    m_houseValue    = createValue(QStringLiteral("Ravenclaw"));
    m_yearValue     = createValue(QStringLiteral("Year 5"));
    m_locationValue = createValue(QStringLiteral("Great Hall"));
    m_galleonValue  = createValue(QStringLiteral("32"));
    m_pointsValue   = createValue(QStringLiteral("120"));
    m_stateValue    = createValue(QStringLiteral("Breakfast"));
    m_courseValue   = createValue(QStringLiteral("Transfiguration"));

    form->addRow(m_nameLabel,     m_nameValue);
    form->addRow(m_houseLabel,    m_houseValue);
    form->addRow(m_yearLabel,     m_yearValue);
    form->addRow(m_locationLabel, m_locationValue);
    form->addRow(m_galleonLabel,  m_galleonValue);
    form->addRow(m_pointsLabel,   m_pointsValue);
    form->addRow(m_stateLabel,    m_stateValue);
    form->addRow(m_courseLabel,   m_courseValue);

    layout->addWidget(m_titleLabel);
    layout->addLayout(form);
    layout->addStretch();
}

void CharacterStatusWidget::retranslateUi()
{
    if (m_titleLabel)   m_titleLabel->setText(TR("title.status"));
    if (m_nameLabel)    m_nameLabel->setText(TR("status.name"));
    if (m_houseLabel)   m_houseLabel->setText(TR("status.house"));
    if (m_yearLabel)    m_yearLabel->setText(TR("status.year"));
    if (m_locationLabel)m_locationLabel->setText(TR("status.location"));
    if (m_galleonLabel) m_galleonLabel->setText(TR("status.galleons"));
    if (m_pointsLabel)  m_pointsLabel->setText(TR("status.housepoints"));
    if (m_stateLabel)   m_stateLabel->setText(TR("status.state"));
    if (m_courseLabel)  m_courseLabel->setText(TR("status.currentclass"));
}

QLabel *CharacterStatusWidget::createValue(const QString &text)
{
    auto *label = new QLabel(text, this);
    label->setObjectName("ValueLabel");
    label->setWordWrap(true);
    return label;
}