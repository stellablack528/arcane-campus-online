#pragma once

#include <QString>
#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    void setDatabaseAvailable(bool available, const QString &reason = {});
    void showFeedback(const QString &text);

signals:
    void loginRequested(const QString &studentName, const QString &houseName);
    void newStudentRequested();

private slots:
    void onLoginClicked();

private:
    void buildUi();
    void retranslateUi();

    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_houseCombo = nullptr;
    QPushButton *m_loginButton = nullptr;
    QPushButton *m_enrollButton = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_houseLabel = nullptr;
    QLabel *m_hintLabel = nullptr;
    bool m_databaseAvailable = true;
    QString m_connectionReason;
    QString m_feedbackText;
};
