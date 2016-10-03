#include "netpage.h"

#include <random>
#include <thread>

#include <QtDebug>
#include <QtNetwork/QNetworkReply.h>

namespace
{
    void waitABit()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(1000, 2000);

        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
    }
}


NetPage::NetPage(QNetworkAccessManager &manager, const QString &url, int commentNum, QObject *parent)
    : QObject(parent)
    , m_netManager(manager)
    , m_commentNum(commentNum)
    , m_url(url)
{
}

void NetPage::load()
{
    const QString& fullTag = m_url + "?page=" + std::to_string(m_commentNum).c_str() + "&view=flat";
    qDebug() << "Downloading page " + fullTag;
    waitABit();
    m_reply = m_netManager.get(QNetworkRequest(QUrl(fullTag)));
    connect(m_reply, SIGNAL(finished()), this, SLOT(finished()));
}

void NetPage::finished()
{
    m_reply->deleteLater();
    if(m_reply->error() != QNetworkReply::NoError)
    {
        errorMessage = m_reply->errorString();
        emit done();
        return;
    }

    try
    {
        Parse(m_reply->readAll());
    }
    catch (const std::runtime_error& err)
    {
        errorMessage = err.what();
    }

    emit done();
}

void NetPage::ParsePrev(const QString& page)
{
    auto start = page.indexOf("b-singlepost-prevnext-link");
    start -= 2;
    while (start > -1 && page[start] != '"')
        start--;

    if (start < 0)
        return;

    auto end = start;
    start--;
    while (start > -1 && page[start] != '"')
        start--;

    prev = page.mid(start + 1, end - start - 1);
}

void NetPage::ParseNext(const QString& page)
{
    auto start = page.indexOf("b-singlepost-prevnext-link");
    if (start == -1)
        return;
    start = page.indexOf("b-singlepost-prevnext-link", start + 1);

    start -= 2;
    while (start > -1 && page[start] != '"')
        start--;

    if (start < 0)
        return;

    auto end = start;
    start--;
    while (start > -1 && page[start] != '"')
        start--;

    next = page.mid(start + 1, end - start - 1);
}

void NetPage::Parse(const QString& page)
{
    {
        const static QString START_TAG("<div class=\"b-singlepost-wrapper\">");

        auto start = page.indexOf(START_TAG);
        if (start == -1)
            throw std::runtime_error("Failed to find article's begining.");

        auto end = page.indexOf("</div>", start);
        if (end == -1)
            throw std::runtime_error("Failed to find article's end.");

        article = page.mid(start + START_TAG.size(), end - (start + START_TAG.size()));
    }

    {
        const static QString START_TAG("Site.page = ");

        auto start = page.indexOf(START_TAG);
        if (start == -1)
            throw std::runtime_error("Failed to find comments's begining.");

        auto end = page.indexOf("\n", start);
        if (end < 2)
            throw std::runtime_error("Failed to find article's end.");

        end--;
        if (page[end] == '\r')
            end--;

        comments = page.mid(start + START_TAG.size(), end - (start + START_TAG.size()));
    }

    ParsePrev(page);
    ParseNext(page);

    const auto lastCommentPageTag = QString("?page=") + std::to_string(m_commentNum + 1).c_str() + "&view=flat";
    lastCommentPage = (-1 == page.indexOf(lastCommentPageTag));
}
