#include "page.h"

#include <QtNetwork/QNetworkReply.h>

Page::Page(QNetworkAccessManager &manager, const QString &url, QObject *parent) : QObject(parent)
{
    m_reply = manager.get(QNetworkRequest(QUrl(url)));
    connect(m_reply, SIGNAL(finished()), this, SLOT(finished()));
}

void Page::finished()
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

void Page::ParsePrev(const QString& page)
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

void Page::ParseNext(const QString& page)
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

void Page::Parse(const QString& page)
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
}
