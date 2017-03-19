#ifndef CALENDAR_H
#define CALENDAR_H

#include <QDate>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class Calendar : public QObject
{
    Q_OBJECT
public:
    Calendar(QNetworkAccessManager& netManager, const QString& name);

signals:
    void pageFound(const QString& number, const QString& name);

private slots:
    void calendarLoaded();

private:
    void query();

    QNetworkAccessManager& m_netManager;
    QNetworkReply* m_reply = nullptr;
    const QString m_name;

    QDate m_date;
    QString getUrl() const;
};

#endif // CALENDAR_H
