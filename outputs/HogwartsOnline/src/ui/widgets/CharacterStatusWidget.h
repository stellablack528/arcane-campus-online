#pragma once

#include <QString>
#include <QWidget>

class QLabel;

class CharacterStatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CharacterStatusWidget(QWidget *parent = nullptr);

public slots:
    void setIdentity(const QString &studentName, const QString &houseName);

private:
    void buildUi();
    QLabel *createValue(const QString &text);

    QLabel *m_nameValue = nullptr;
    QLabel *m_houseValue = nullptr;
    QLabel *m_yearValue = nullptr;
    QLabel *m_locationValue = nullptr;
    QLabel *m_galleonValue = nullptr;
    QLabel *m_pointsValue = nullptr;
    QLabel *m_stateValue = nullptr;
    QLabel *m_courseValue = nullptr;
};
