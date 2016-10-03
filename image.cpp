#include "image.h"

#include <random>
#include <thread>

#include <QtDebug>
#include <QDir>
#include <QFile>
#include <QtNetwork/QNetworkReply.h>


namespace
{
    void waitABit()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(1000, 2000);

        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
    }
}

Image::Image(QNetworkAccessManager &netManager, const QString &storage, const QString& filename, const QString& url, QObject *parent)
    : QObject(parent)
    , m_netManager(netManager)
    , m_storage(storage)
    , m_filename(filename)
    , m_url(url)
{
}

void Image::load()
{
    started = true;

    if (saved())
    {
        finished = true;
        emit done();
        return;
    }

    qDebug() << "Downloading image " + m_url;
    waitABit();
    m_reply = m_netManager.get(QNetworkRequest(QUrl(m_url)));
    connect(m_reply, SIGNAL(finished()), this, SLOT(finishedDownload()));
}

void Image::finishedDownload()
{
    m_reply->deleteLater();
    if(m_reply->error() != QNetworkReply::NoError)
    {
        errorMessage = m_reply->errorString();
        emit done();
        return;
    }

    QDir storage(m_storage);
    storage.mkdir(".");
    const QString& filePath = storage.absolutePath() + QDir::separator() + m_filename;
    QFile localFile(filePath);
    if (!localFile.open(QIODevice::WriteOnly))
    {
        errorMessage = "Failed to save image to " + filePath;
        emit done();
        return;
    }

    localFile.write(m_reply->readAll());
    localFile.close();

    finished = true;
    emit done();
}

bool Image::saved()
{
    QDir storage(m_storage);
    const QString& filePath = storage.absolutePath() + QDir::separator() + m_filename;
    QFile localFile(filePath);

    return !storage.mkdir(".") && localFile.exists();
}
