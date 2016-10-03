#ifndef NETPAGE_H
#define NETPAGE_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class NetPage : public QObject
{
    Q_OBJECT
public:
    explicit NetPage(QNetworkAccessManager& manager, const QString& url, int m_commentNum = 1, QObject *parent = 0);

    QString errorMessage;

    QString article;
    QString comments;

    QString prev;
    QString next;

    bool lastCommentPage = false;

signals:
    void done();

public slots:

private slots:
    void finished();

private:
    const int m_commentNum;
    QNetworkReply* m_reply;

    void Parse(const QString& page);
    void ParsePrev(const QString& page);
    void ParseNext(const QString& page);
};

#endif // NETPAGE_H
