#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QComboBox;

class WelcomeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeWindow(QWidget *parent = nullptr);
    void retranslateUi();

signals:
    void startRequested();

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void applyTheme();

    QLabel *m_crestLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QLabel *m_quoteLabel = nullptr;
    QLabel *m_langLabel = nullptr;
    QComboBox *m_langCombo = nullptr;
    QPushButton *m_startButton = nullptr;
};
