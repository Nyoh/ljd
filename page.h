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
    QString article;
    QVector<QJsonObject> rawComments;

    std::atomic<bool> started{false};
    std::atomic<bool> finished{false};

    explicit Page(QNetworkAccessManager& netManager, const QString& storage, const QString& name, const QString& number, QObject *parent = 0);
    void load();

signals:
    void finishedPage(int page, bool halted);
    void finishedAll();

public slots:

private slots:
    void loadedFromNet();

private:
    bool loadFirstFromStorage();

    void save();

    const QString m_storage;
    const QString m_name;
    const QString m_number;
    int m_commentPagesLoaded = 0;
    bool m_commentsFinished = false;
    NetPage* m_netPage = nullptr;
    QNetworkAccessManager& m_netManager;
};

#endif // PAGE_H
