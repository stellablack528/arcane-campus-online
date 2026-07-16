#pragma once

#include <QWidget>
#include <QString>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QSplitter;

class LocationBrowserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LocationBrowserWidget(QWidget *parent = nullptr);
    void retranslateUi();

signals:
    void locationSelected(const QString &locationId, const QString &nameEn);
    void travelRequested(const QString &locationId);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void rebuildList();

    QLabel *m_titleLabel = nullptr;
    QListWidget *m_locationList = nullptr;
    QLabel *m_detailTitle = nullptr;
    QLabel *m_detailDesc = nullptr;
    QLabel *m_detailOnline = nullptr;
    QLabel *m_detailNpcs = nullptr;

    QString m_currentId;
};
