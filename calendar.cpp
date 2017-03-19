#include "calendar.h"
#include "page.h"

#include <QNetworkAccessManager>
#include <QtNetwork/QNetworkReply.h>

Calendar::Calendar(QNetworkAccessManager &netManager, const QString &name)
    : m_netManager(netManager)
    , m_name(name)
    , m_date(QDateTime::currentDateTime().date())
{
    query();
}

void Calendar::calendarLoaded()
{
    m_reply->deleteLater();
    try
    {
        if(m_reply->error() != QNetworkReply::NoError)
            throw "Failed to download calendar for " + m_name + ". " + m_reply->errorString();

        qDebug() << "Downloaded: " + m_reply->url().toString();
        const QString& page = m_reply->readAll();

        QRegExp rx(m_name + "\\.livejournal\\.com/([0-9]+)\\.html");
        QStringList list;
        int pos = 0;

        while ((pos = rx.indexIn(page, pos)) != -1) {
            emit pageFound(rx.cap(1), m_name);
            pos += rx.matchedLength();
        }

        m_date = m_date.addMonths(-1);
        const QString& newUrl = getUrl();
        auto start = page.indexOf(newUrl);
        if (start != -1)
            query();
    }
    catch (const QString& err)
    {
        qWarning() << err;
        //        query(m_articleReply, articleUrl(m_name, m_number));
        //        connect(m_articleReply, SIGNAL(finished()), this, SLOT(articleFromNet()));
    }
}

void Calendar::query()
{
    const QString& url = getUrl();

    qDebug() << "Downloading " + url;
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent" , "Mozilla/5.0 (Android 5.1; Mobile; rv:52.0) Gecko/52.0 Firefox/52.0");
    m_reply = m_netManager.get(request);

    connect(m_reply, SIGNAL(finished()), this, SLOT(calendarLoaded()), Qt::QueuedConnection);
}

QString Calendar::getUrl() const
{
    const QString year = QString::number(m_date.year());
    const QString month = (m_date.month() < 10 ? "0" : "") + QString::number(m_date.month());

    return "http://" + m_name + LJ_TAG + year + "/" + month + "/";
}
