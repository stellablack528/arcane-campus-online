#pragma once

#include <QDialog>

class QLineEdit;

class ApiKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ApiKeyDialog(QWidget *parent = nullptr);

private slots:
    void onSaveClicked();

private:
    void buildUi();

    QLineEdit *m_keyEdit = nullptr;
};
