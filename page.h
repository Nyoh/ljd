#ifndef PAGE_H
#define PAGE_H

#include <atomic>

#include <QJsonObject>
#include <QObject>
#include <QVector>

const static QString LJ_TAG = QString(".livejournal.com/");

class QNetworkAccessManager;
class NetPage;

class Page : public QObject
{
    Q_OBJECT
public:
    struct Info
    {
        QString article;
        QVector<QJsonObject> rawComments;
    } info;

    std::atomic<bool> loading{false};

    explicit Page(const QString& storage, const QString& name, const QString& number, QObject *parent = 0);
    void load();

signals:
    void finished(int page);

public slots:

private slots:
    void loadedFromNet();

private:
    bool loadFirstFromStorage();
    void postProcess();

    void save();

    const QString m_storage;
    const QString m_name;
    const QString m_number;
    int m_commentPagesLoaded = 0;
    bool m_commentsFinished = false;
    NetPage* m_netPage = nullptr;
    QNetworkAccessManager* m_netManager = nullptr;
};

#endif // PAGE_H
