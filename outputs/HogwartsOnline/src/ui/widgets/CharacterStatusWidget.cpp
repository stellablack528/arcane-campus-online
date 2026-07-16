#include "ui/widgets/CharacterStatusWidget.h"
#include "core/LanguageManager.h"

#include <QEvent>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

CharacterStatusWidget::CharacterStatusWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("CharacterStatusWidget");
    buildUi();
    retranslateUi();
}

void CharacterStatusWidget::setIdentity(const QString &studentName, const QString &houseName)
{
    m_nameValue->setText(studentName);
    m_houseValue->setText(houseName);
}

void CharacterStatusWidget::setWandInfo(const QString &wood, const QString &core,
                                        int lengthInches, const QString &flexibility)
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);
    const QString text = isCn
        ? QString("🌳 %1 · %2 · %3″ · %4").arg(wood, core).arg(lengthInches).arg(flexibility)
        : QString("🌳 %1 · %2 · %3\" · %4").arg(wood, core).arg(lengthInches).arg(flexibility);
    m_wandValue->setText(text);
}

void CharacterStatusWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("PanelTitle");

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
    m_wandValue = createValue("—");

    m_nameLabel = new QLabel(this);
    m_houseLabel = new QLabel(this);
    m_yearLabel = new QLabel(this);
    m_locationLabel = new QLabel(this);
    m_galleonLabel = new QLabel(this);
    m_pointsLabel = new QLabel(this);
    m_stateLabel = new QLabel(this);
    m_courseLabel = new QLabel(this);
    m_wandLabel = new QLabel(this);

    form->addRow(m_nameLabel, m_nameValue);
    form->addRow(m_houseLabel, m_houseValue);
    form->addRow(m_yearLabel, m_yearValue);
    form->addRow(m_wandLabel, m_wandValue);
    form->addRow(m_locationLabel, m_locationValue);
    form->addRow(m_galleonLabel, m_galleonValue);
    form->addRow(m_pointsLabel, m_pointsValue);
    form->addRow(m_stateLabel, m_stateValue);
    form->addRow(m_courseLabel, m_courseValue);

    layout->addWidget(m_titleLabel);
    layout->addLayout(form);
    layout->addStretch();
}

void CharacterStatusWidget::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);
    m_titleLabel->setText(isCn ? "学生状态" : "Student Status");
    m_nameLabel->setText(isCn ? "姓名" : "Name");
    m_houseLabel->setText(isCn ? "学院" : "House");
    m_yearLabel->setText(isCn ? "年级" : "Year");
    m_wandLabel->setText(isCn ? "魔杖" : "Wand");
    m_locationLabel->setText(isCn ? "位置" : "Location");
    m_galleonLabel->setText(isCn ? "加隆" : "Galleons");
    m_pointsLabel->setText(isCn ? "学院积分" : "House Points");
    m_stateLabel->setText(isCn ? "状态" : "State");
    m_courseLabel->setText(isCn ? "当前课程" : "Current Class");
}

void CharacterStatusWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}

QLabel *CharacterStatusWidget::createValue(const QString &text)
{
    auto *label = new QLabel(text, this);
    label->setObjectName("ValueLabel");
    label->setWordWrap(true);
    return label;
}
