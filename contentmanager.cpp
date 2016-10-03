#include "contentmanager.h"

#include <QNetworkAccessManager>

ContentManager::ContentManager()
    : QThread()
{
    moveToThread(this);
    connect(this, SIGNAL(newJob()), this, SLOT(workTick()), Qt::QueuedConnection);
    start();
}

QSharedPointer<Page> ContentManager::getPage(const QString &storage, const QString &name, const QString &number)
{
    const QString& key = storage + "?@?" + name + "?@?" + number;
    while(m_manager.load() == nullptr);

    std::unique_lock<std::mutex> lock(m_guard);
    QSharedPointer<Page>& result = m_pages[key];
    if (!result)
    {
        result = QSharedPointer<Page>::create(*m_manager, storage, name, number);
        result->moveToThread(this);

        connect(result.data(), &Page::finishedPage, [result, this](int page, bool halted){
            finishedPage(result, page, halted);});

        m_pageLine.push_back(result);
        lock.unlock();
        kickWorker();
    }

    return result;
}

ContentManager::~ContentManager()
{
    stop = true;
}

void ContentManager::finishedPage(QSharedPointer<Page> page, int, bool halted)
{
    std::unique_lock<std::mutex> lock(m_guard);
    if (halted)
    {
        m_pageLine.push_front(page);
        emit newJob();
    }
}

void ContentManager::run()
{
    m_manager = new QNetworkAccessManager(this);
    exec();
}

void ContentManager::kickWorker()
{
    emit newJob();
}

void ContentManager::workTick()
{
    stop = false;

    std::unique_lock<std::mutex> lock(m_guard);
    if (!m_pageLine.isEmpty() && !stop)
    {
        auto workingPage = m_pageLine.front();
        m_pageLine.pop_front();
        lock.unlock();

        workingPage->start();

        lock.lock();
    }

    stop = true;
}
