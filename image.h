#ifndef IMAGE_H
#define IMAGE_H

#include <atomic>

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class Image : public QObject
{
    Q_OBJECT
public:
    explicit Image(QNetworkAccessManager& netManager, const QString& storage, const QString& filename, const QString& url, QObject *parent = 0);
    void load();

    const QString m_storage;
    const QString m_filename;
    const QString m_url;

signals:
    void done(const QString& url, bool failed);

private slots:
    void finishedDownload();

private:
    bool saved();

    QNetworkAccessManager& m_netManager;
    QNetworkReply* m_reply;
};

#endif // IMAGE_H
