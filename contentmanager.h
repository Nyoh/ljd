#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <mutex>

#include <QHash>
#include <QQueue>
#include <QSharedPointer>
#include <QThread>

#include "image.h"
#include "page.h"

class ContentManager : public QThread
{
    Q_OBJECT
public:
    explicit ContentManager();

    QSharedPointer<Page> getPage(const QString& storage, const QString& name, const QString& number);
    std::atomic<bool> stop{true};

    ~ContentManager();

signals:
    void newJob();

public slots:

private slots:
    void finishedPage(QSharedPointer<Page> page, int, bool halted);
    void workTick();

private:
    void run() override;
    void kickWorker();

    std::mutex m_guard;
    std::atomic<QNetworkAccessManager*> m_manager{nullptr};

    QHash<QString, QSharedPointer<Page>> m_pages;
    QQueue<QSharedPointer<Page>> m_pageLine;
};

#endif // DOWNLOADMANAGER_H
