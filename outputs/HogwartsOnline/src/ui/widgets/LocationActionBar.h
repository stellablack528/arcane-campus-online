#pragma once

#include <QWidget>

class QPushButton;

class LocationActionBar : public QWidget
{
    Q_OBJECT

public:
    explicit LocationActionBar(QWidget *parent = nullptr);
    void retranslateUi();

signals:
    void moveToLocationRequested(const QString &locationId);
    void joinActivityRequested(const QString &activityId);
    void studyRequested(const QString &locationId);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();

    QPushButton *m_greatHall = nullptr;
    QPushButton *m_classroom = nullptr;
    QPushButton *m_library = nullptr;
    QPushButton *m_courtyard = nullptr;
    QPushButton *m_hogsmeade = nullptr;
};
