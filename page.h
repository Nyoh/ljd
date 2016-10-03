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
    QString comments;

    QString prev;
    QString next;

signals:
    void done();

public slots:

private slots:
    void finished();

private:
    void Parse(const QString& page);

    QNetworkReply* m_reply;
    void ParsePrev(const QString& page);
    void ParseNext(const QString& page);
};

#endif // PAGE_H
