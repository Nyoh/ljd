#ifndef PAGE_H
#define PAGE_H

#include <atomic>

#include <QJsonObject>
#include <QObject>

const static QString LJ_TAG = QString(".livejournal.com/");

class QNetworkAccessManager;
class QNetworkReply;

class Page : public QObject
{
    Q_OBJECT
public:
    QString url;
    QString article;
    QJsonObject rawComments;
    QString prev;
    QString next;

    explicit Page(QNetworkAccessManager& netManager, const QString& storage, const QString& name, const QString& number);
    void load();

    std::atomic<bool> commentsDone{false};
    std::atomic<bool> articleDone{false};

    void signalIfFinished();

signals:
    void finished();

private slots:
    void articleFromNet();
    void commentsFromNet();

private:
    bool loadFromStorage();
    void query(QNetworkReply*& reply, const QString& url);

    void save();

    QString commentsUrl() const;
    const QString m_storage;
    const QString m_name;
    const QString m_number;
    unsigned m_commentPage = 1;

    QNetworkReply* m_articleReply = nullptr;
    QNetworkReply* m_commentsReply = nullptr;
    QNetworkAccessManager& m_netManager;

    void parsePrev(const QString &page);
    void parseNext(const QString &page);
    void parse(const QString &page);
};

#endif // PAGE_H
