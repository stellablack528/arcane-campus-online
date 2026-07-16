#pragma once

#include <QString>
#include <QWidget>

class QLabel;

class CharacterStatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CharacterStatusWidget(QWidget *parent = nullptr);
    void retranslateUi();

public slots:
    void setIdentity(const QString &studentName, const QString &houseName);
    void setWandInfo(const QString &wood, const QString &core, int lengthInches,
                     const QString &flexibility);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    QLabel *createValue(const QString &text);

    QLabel *m_titleLabel = nullptr;
    QLabel *m_nameValue = nullptr;
    QLabel *m_houseValue = nullptr;
    QLabel *m_yearValue = nullptr;
    QLabel *m_locationValue = nullptr;
    QLabel *m_galleonValue = nullptr;
    QLabel *m_pointsValue = nullptr;
    QLabel *m_stateValue = nullptr;
    QLabel *m_courseValue = nullptr;

    QLabel *m_nameLabel = nullptr;
    QLabel *m_houseLabel = nullptr;
    QLabel *m_yearLabel = nullptr;
    QLabel *m_locationLabel = nullptr;
    QLabel *m_galleonLabel = nullptr;
    QLabel *m_pointsLabel = nullptr;
    QLabel *m_stateLabel = nullptr;
    QLabel *m_courseLabel = nullptr;
    QLabel *m_wandLabel = nullptr;
    QLabel *m_wandValue = nullptr;
};
