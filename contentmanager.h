#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <mutex>
#include <thread>

#include <QObject>
#include <QHash>
#include <QQueue>

#include "image.h"
#include "page.h"

class ContentManager : public QObject
{
    Q_OBJECT
public:
    explicit ContentManager(QObject *parent = 0);

    const Page* getPage(const QString& storage, const QString& name, const QString& number);
    std::atomic<bool> stop{true};

    ~ContentManager();

signals:

public slots:

private slots:
    void finishedPage(int, bool halted);

private:
    void kickWorker();
    void workCycle();

    std::thread m_thread;
    std::mutex m_guard;
    QNetworkAccessManager* m_manager;

    QHash<QString, Page*> m_pages;
    QQueue<Page*> m_pageLine;
};

#endif // DOWNLOADMANAGER_H
