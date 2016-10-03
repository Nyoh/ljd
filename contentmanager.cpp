#include "contentmanager.h"

#include <QNetworkAccessManager>

ContentManager::ContentManager(QObject *parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
}

const Page* ContentManager::getPage(const QString &storage, const QString &name, const QString &number)
{
    const QString& key = storage + "?@?" + name + "?@?" + number;
    std::unique_lock<std::mutex> lock(m_guard);
    Page*& result = m_pages[key];
    if (!result)
    {
        result = new Page(*m_manager, storage, name, number, this);
        connect(result, SIGNAL(finishedPage(int, bool)), this, SLOT(finishedPage(int, bool)) );
        m_pageLine.push_back(result);
        kickWorker();
    }

    return result;
}

ContentManager::~ContentManager()
{
    stop = true;
    if (m_thread.joinable())
        m_thread.join();
}

void ContentManager::finishedPage(int, bool halted)
{
    std::unique_lock<std::mutex> lock(m_guard);
    if (!halted)
        m_pageLine.pop_front();
}

void ContentManager::kickWorker()
{
    if (!stop)
        return;

    if (m_thread.joinable())
        m_thread.join();

    m_thread = std::thread([this](){workCycle();});
}

void ContentManager::workCycle()
{
    stop = false;

    std::unique_lock<std::mutex> lock(m_guard);
    while (!m_pageLine.isEmpty() && !stop)
    {
        Page* workingPage = m_pageLine.front();
        lock.unlock();

        workingPage->load();

        lock.lock();
    }

    stop = true;
}
