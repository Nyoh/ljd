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

void Page::Parse(const QString& page)
{
    const static QString START_TAG("<div class=\"b-singlepost-wrapper\">");

    auto start = page.indexOf(START_TAG);
    if (start == -1)
        throw std::runtime_error("Failed to find article's begining.");

    auto end = page.indexOf("</div>", start);
    if (end == -1)
        throw std::runtime_error("Failed to find article's end.");

    //article = page.mid(start + START_TAG.size(), end - (start + START_TAG.size()));
    article  = page;
}
