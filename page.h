#ifndef PAGE_H
#define PAGE_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class Page : public QObject
{
    Q_OBJECT
public:
    explicit Page(QNetworkAccessManager& manager, const QString& url, QObject *parent = 0);

    QString errorMessage;

    QString article;

signals:
    void done();

public slots:

private slots:
    void finished();

private:
    void Parse(const QString& page);

    QNetworkReply* m_reply;
};

#endif // PAGE_H
