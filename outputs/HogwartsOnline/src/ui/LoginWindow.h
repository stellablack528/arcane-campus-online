#pragma once

#include <QString>
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginRequested(const QString &studentName, const QString &houseName);

private slots:
    void onLoginClicked();

private:
    void buildUi();

    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_houseCombo = nullptr;
    QPushButton *m_loginButton = nullptr;
};
