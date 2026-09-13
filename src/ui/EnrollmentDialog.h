#pragma once

#include "application/dto/CampusDTO.hpp"

#include <QDialog>
#include <QHash>
#include <QString>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QStackedWidget;

class EnrollmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnrollmentDialog(QWidget *parent = nullptr);

    // 在 accept() 后调用，构建完整的入学请求 DTO。
    [[nodiscard]] arcane::application::dto::EnrollmentRequestDTO buildRequest() const;

private:
    void buildUi();
    void loadWandData();
    // 根据当前魔杖选择 + 性格倾向，生成"你的魔杖为什么选择你"描述。
    [[nodiscard]] QString generateWandLore() const;

    QStackedWidget *m_pages = nullptr;

    // Page 1: 角色信息
    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_nicknameEdit = nullptr;
    QComboBox *m_houseCombo = nullptr;
    QComboBox *m_genderCombo = nullptr;
    QLineEdit *m_hairColorEdit = nullptr;
    QLineEdit *m_eyeColorEdit = nullptr;
    QComboBox *m_bloodStatusCombo = nullptr;
    QComboBox *m_personalityCombo = nullptr;

    // Page 2: 魔杖选择
    QComboBox *m_woodCombo = nullptr;
    QComboBox *m_coreCombo = nullptr;
    QDoubleSpinBox *m_lengthSpin = nullptr;
    QComboBox *m_flexibilityCombo = nullptr;
    QComboBox *m_handleStyleCombo = nullptr;
    QComboBox *m_finishCombo = nullptr;
    QComboBox *m_colorToneCombo = nullptr;
    QLineEdit *m_engravingEdit = nullptr;

    // Page 3: 动物 + 魔杖描述
    QComboBox *m_petCombo = nullptr;
    QLabel *m_wandLoreLabel = nullptr;

    // 魔杖数据缓存：id → traits（用于生成描述）
    QHash<QString, QString> m_woodTraits;
    QHash<QString, QString> m_coreTraits;
};
