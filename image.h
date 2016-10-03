#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class Image : public QObject
{
    Q_OBJECT
public:
    explicit Image(QNetworkAccessManager& netManager, const QString& storage, const QString& filename, const QString& url, QObject *parent = 0);
    void load();

signals:
    void done();

public slots:

private slots:
    void finished();

private:
    bool saved();

    QNetworkAccessManager& m_netManager;
    QNetworkReply* m_reply;

public:
    QString errorMessage;

    const QString m_storage;
    const QString m_filename;
    const QString m_url;
};

#endif // IMAGE_H
