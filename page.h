#ifndef PAGE_H
#define PAGE_H

#include <QObject>

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
    } info;

    explicit Page(const QString& storage, const QString& name, const QString& number, QObject *parent = 0);

signals:
    void finished(int page);

public slots:

private slots:
    void netPageLoaded();

private:
    bool loadFirstFromStorage();
    void loadFirstFromNet();

    const QString m_storage;
    const QString m_name;
    const QString m_number;
    NetPage* m_netPage = nullptr;
    QNetworkAccessManager* m_netManager = nullptr;
};

#endif // PAGE_H
