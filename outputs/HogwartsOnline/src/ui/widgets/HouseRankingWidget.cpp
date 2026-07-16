#include "ui/widgets/HouseRankingWidget.h"
#include "core/LanguageManager.h"

#include <QEvent>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

HouseRankingWidget::HouseRankingWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("HouseRankingWidget");
    buildUi();
    retranslateUi();
}

void HouseRankingWidget::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("PanelTitle");

    m_list = new QListWidget(this);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_list, 1);
}

void HouseRankingWidget::retranslateUi()
{
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);
    m_titleLabel->setText(isCn ? "🏆 学院杯" : "🏆 House Cup");
    refreshRanking();
}

void HouseRankingWidget::refreshRanking()
{
    if (!m_list) return;
    m_list->clear();
    auto &lm = LanguageManager::instance();
    const bool isCn = (lm.currentLanguage() == Language::Chinese);

    if (isCn) {
        m_list->addItem("1.  🦅  拉文克劳       145");
        m_list->addItem("2.  🦁  格兰芬多       132");
        m_list->addItem("3.  🦡  赫奇帕奇       118");
        m_list->addItem("4.  🐍  斯莱特林       109");
    } else {
        m_list->addItem("1.  🦅  Ravenclaw       145");
        m_list->addItem("2.  🦁  Gryffindor      132");
        m_list->addItem("3.  🦡  Hufflepuff      118");
        m_list->addItem("4.  🐍  Slytherin       109");
    }
}

void HouseRankingWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}
