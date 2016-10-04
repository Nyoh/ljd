#include "downloader.h"

#include <QNetworkAccessManager>

Downloader::Downloader(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

void Downloader::get(const QString &storage, const QString &url, const QString &filename)
{
    const QString& key = storage + "?@?" + url + "?@?" + filename;
    QSharedPointer<Image>& image = m_images[key];
    if (!image)
    {
        image = QSharedPointer<Image>::create(*m_manager, storage, filename, url);
//        connect(result.data(), SIGNAL(done(QString, bool)), this, SLOT(imageDownloaded(QString, bool)));
        image->load();
    }
}

QNetworkAccessManager *Downloader::getNetwork()
{
    return m_manager;
}
