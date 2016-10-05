#include "image.h"

#include <random>
#include <thread>

#include <QtDebug>
#include <QDir>
#include <QFile>
#include <QtNetwork/QNetworkReply.h>

Image::Image(QNetworkAccessManager &netManager, const QString &storage, const QString& filename, const QString& url, QObject *parent)
    : QObject(parent)
    , m_storage(storage)
    , m_filename(filename)
    , m_url(url)
    , m_netManager(netManager)
{
}

void Image::load()
{
    if (saved())
    {
        emit done(m_url, false);
        return;
    }

    m_reply = m_netManager.get(QNetworkRequest(QUrl(m_url)));
    connect(m_reply, SIGNAL(finished()), this, SLOT(finishedDownload()));
}

void Image::finishedDownload()
{
    m_reply->deleteLater();
    if(m_reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Failed to download image:" + m_url + "   " + m_reply->errorString();
        emit done(m_url, true);
        return;
    }

    QDir storage(m_storage);
    storage.mkdir(".");
    const QString& filePath = storage.absolutePath() + QDir::separator() + m_filename;
    QFile localFile(filePath);
    if (!localFile.open(QIODevice::WriteOnly))
    {
        qCritical() << "Failed to save image to " + filePath;
        emit done(m_url, true);
        return;
    }

    localFile.write(m_reply->readAll());
    localFile.close();

    qDebug() << "Downloaded image " + m_url;
    emit done(m_url, false);
}

bool Image::saved()
{
    QDir storage(m_storage);
    const QString& filePath = storage.absolutePath() + QDir::separator() + m_filename;
    QFile localFile(filePath);

    return !storage.mkdir(".") && localFile.exists();
}
