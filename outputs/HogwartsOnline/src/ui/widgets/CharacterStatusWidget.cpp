#include "ui/widgets/CharacterStatusWidget.h"

#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

CharacterStatusWidget::CharacterStatusWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("CharacterStatusWidget");
    buildUi();
}

void CharacterStatusWidget::setIdentity(const QString &studentName, const QString &houseName)
{
    m_nameValue->setText(studentName);
    m_houseValue->setText(houseName);
}

void CharacterStatusWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *title = new QLabel("Student Status", this);
    title->setObjectName("PanelTitle");

    auto *form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignLeft);
    form->setFormAlignment(Qt::AlignTop);
    form->setVerticalSpacing(10);

    m_nameValue = createValue("New Student");
    m_houseValue = createValue("Ravenclaw");
    m_yearValue = createValue("Year 5");
    m_locationValue = createValue("Great Hall");
    m_galleonValue = createValue("32");
    m_pointsValue = createValue("120");
    m_stateValue = createValue("Breakfast");
    m_courseValue = createValue("Transfiguration");

    form->addRow("Name", m_nameValue);
    form->addRow("House", m_houseValue);
    form->addRow("Year", m_yearValue);
    form->addRow("Location", m_locationValue);
    form->addRow("Galleons", m_galleonValue);
    form->addRow("House Points", m_pointsValue);
    form->addRow("State", m_stateValue);
    form->addRow("Current Class", m_courseValue);

    layout->addWidget(title);
    layout->addLayout(form);
    layout->addStretch();
}

QLabel *CharacterStatusWidget::createValue(const QString &text)
{
    auto *label = new QLabel(text, this);
    label->setObjectName("ValueLabel");
    label->setWordWrap(true);
    return label;
}
