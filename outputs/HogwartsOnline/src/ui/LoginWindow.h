#pragma once

#include <QString>
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QLabel;
class QSlider;

// 表示魔杖信息的简单结构
struct WandInfo {
    QString wood;
    QString core;
    int lengthInches = 11;        // 长度（英寸）
    QString flexibility;          // 柔韧度
};

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    void retranslateUi();

signals:
    // 玩家身份 + 魔杖信息
    void loginRequested(const QString &studentName,
                        const QString &houseName,
                        const WandInfo &wand);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onNextClicked();   // 下一步
    void onBackClicked();   // 上一步
    void onLoginClicked();  // 最终进入
    void onWandChanged();   // 魔杖属性变化（更新预览）

private:
    void buildUi();
    void buildStepName();    // 第一步：姓名
    void buildStepHouse();   // 第二步：学院
    void buildStepWand();    // 第三步：魔杖
    void gotoStep(int step);
    void updateWandPreview();

    // UI 组件
    QStackedWidget *m_stack = nullptr;
    QLabel *m_stepIndicator = nullptr;
    QPushButton *m_backButton = nullptr;
    QPushButton *m_nextButton = nullptr;
    QPushButton *m_loginButton = nullptr;

    // 第一步：姓名
    QLineEdit *m_nameEdit = nullptr;

    // 第二步：学院
    QComboBox *m_houseCombo = nullptr;

    // 第三步：魔杖
    QComboBox *m_woodCombo = nullptr;
    QComboBox *m_coreCombo = nullptr;
    QSlider *m_lengthSlider = nullptr;
    QLabel *m_lengthValueLabel = nullptr;
    QComboBox *m_flexCombo = nullptr;

    int m_currentStep = 0;
};
