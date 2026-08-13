#pragma once

#include <QString>
#include <QWidget>

class LocationActionBar : public QWidget
{
    Q_OBJECT

public:
    explicit LocationActionBar(QWidget *parent = nullptr);

signals:
    void moveToLocationRequested(const QString &locationId);
    void joinActivityRequested(const QString &activityId);
    void studyRequested(const QString &locationId);

private:
    void buildUi();
};
