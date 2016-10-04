#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QHash>
#include <QSharedPointer>
#include <QObject>

#include "image.h"
#include "page.h"

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = 0);

    void get(const QString& storage, const QString& url, const QString& filename);
    QNetworkAccessManager* getNetwork();

private:
    QNetworkAccessManager* m_manager;
    QHash<QString, QSharedPointer<Image>> m_images;
};

#endif // DOWNLOADMANAGER_H
