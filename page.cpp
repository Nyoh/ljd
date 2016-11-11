#include "page.h"

#include <random>
#include <thread>

#include <QtDebug>
#include <QDir>
#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork/QNetworkReply.h>

namespace
{
    QString articleFileName(const QString& storage, const QString& name, const QString& number)
    {
        return storage + QDir::separator() + name + number + ".html";
    }

    QString partsFileName(const QString& storage, const QString& name, const QString& number)
    {
        return storage + QDir::separator() + name + number + ".json";
    }

    void waitABit()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(100, 200);

        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
    }
}



Page::Page(QNetworkAccessManager& netManager, const QString& storage, const QString& name, const QString& number)
    : QObject()
    , m_storage(storage)
    , m_name(name)
    , m_number(number)
    , m_netManager(netManager)
{
    url = "http://" + name + LJ_TAG + number + ".html";
}

void Page::query(QNetworkReply *&reply, const QString& url)
{
    waitABit();
    qDebug() << "Downloading " + url;
    reply = m_netManager.get(QNetworkRequest(QUrl(url)));
}

void Page::load()
{
    if (!loadFromStorage())
    {
        query(m_articleReply, url);
        connect(m_articleReply, SIGNAL(finished()), this, SLOT(articleFromNet()), Qt::QueuedConnection);

        query(m_commentsReply, commentsUrl());
        connect(m_commentsReply, SIGNAL(finished()), this, SLOT(commentsFromNet()), Qt::QueuedConnection);
    }
}

void Page::signalIfFinished()
{
    if (commentsDone && articleDone)
        emit finished();
}

bool Page::loadFromStorage()
{
    QDir storageDir(m_storage);

    if (storageDir.mkdir("."))
        return false;

    {
        QFile partsFile(partsFileName(m_storage, m_name, m_number));
        if (!partsFile.open(QIODevice::ReadOnly))
        {
            qCritical() << "Failed to read the file " + partsFileName(m_storage, m_name, m_number);
            return false;
        }

        QJsonParseError error;
        QJsonObject root = QJsonDocument::fromJson(partsFile.readAll(), &error).object();
        rawComments = root["raw_comments"].toObject();
        if (error.error != QJsonParseError::NoError)
        {
            qCritical() << "Failed to parse the file " + partsFileName(m_storage, m_name, m_number);
            return false;
        }

        prev = root["prev"].toString();
        next = root["next"].toString();
    }

    {
        QFile articleFile(articleFileName(m_storage, m_name, m_number));
        if (!articleFile.open(QIODevice::ReadOnly))
        {
            qCritical() << "Failed to read the file " + articleFileName(m_storage, m_name, m_number);
            return false;
        }

        QTextStream stream(&articleFile);
        stream.setGenerateByteOrderMark(true);
        stream.setCodec("UTF-8");
        article = stream.readAll();
    }

    emit finished();
    commentsDone = true;
    articleDone = true;
    return true;
}

void Page::articleFromNet()
{
    m_articleReply->deleteLater();
    try
    {
        if(m_articleReply->error() != QNetworkReply::NoError)
            throw "Failed to download " + url + ". " + m_articleReply->errorString();

        qDebug() << "Downloaded: " + url;
        parse(m_articleReply->readAll());

        articleDone = true;
        save();
    }
    catch (const QString& err)
    {
        qWarning() << err;
        //        query(m_articleReply, articleUrl(m_name, m_number));
        //        connect(m_articleReply, SIGNAL(finished()), this, SLOT(articleFromNet()));
    }
}

void Page::commentsFromNet()
{
    m_commentsReply->deleteLater();
    try
    {
        if(m_commentsReply->error() != QNetworkReply::NoError)
            throw "Failed to download " + commentsUrl() + ". " + m_commentsReply->errorString();

        qDebug() << "Downloaded: " + commentsUrl();

        const QString reply(m_commentsReply->readAll());
        const QJsonDocument& pageJson = QJsonDocument::fromJson(reply.toUtf8());
        const QJsonObject& partsObject = pageJson.object();
        QJsonArray rawCommentsArray = partsObject["comments"].toArray();
        for (const auto& commentJson : rawCommentsArray)
        {
            QJsonObject rawComment = commentJson.toObject();
            const QString commentNum = QString::number(rawComment["dtalkid"].toInt());
            if (rawComments.find(commentNum) == rawComments.end())
                rawComments[commentNum] = rawComment;
            else
            {
                commentsDone = true;
                save();
                return;
            }
        }

        m_commentPage++;
        query(m_commentsReply, commentsUrl());
        connect(m_commentsReply, SIGNAL(finished()), this, SLOT(commentsFromNet()));
    }
    catch (const QString& err)
    {
        qWarning() << err;
        //query(m_commentsReply, commentsUrl(m_name, m_number));
        //connect(m_commentsReply, SIGNAL(finished()), this, SLOT(commentsFromNet()));
        return;
    }

}

void Page::save()
{
    if (!commentsDone || !articleDone)
        return;

    {
        QFile articleFile(articleFileName(m_storage, m_name, m_number));
        if (!articleFile.open(QIODevice::WriteOnly))
        {
            qCritical() << "Failed to write to file " + articleFileName(m_storage, m_name, m_number);
            return;
        }

        QTextStream stream(&articleFile);
        stream.setGenerateByteOrderMark(true);
        stream.setCodec("UTF-8");
        stream << article.toUtf8();
    }

    {
        QFile partsFile(partsFileName(m_storage, m_name, m_number));
        if (!partsFile.open(QIODevice::WriteOnly))
        {
            qCritical() << "Failed to write to file " + partsFileName(m_storage, m_name, m_number);
            return;
        }

        QJsonObject root;
        root["raw_comments"] = rawComments;
        root["prev"] = prev;
        root["next"] = next;

        QJsonDocument saveDoc(root);
        partsFile.write(saveDoc.toJson());
    }

    emit finished();
}


void Page::parsePrev(const QString& page)
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

void Page::parseNext(const QString& page)
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

void Page::parse(const QString& page)
{
    {
        const static QString START_TAG("<div class=\"b-singlepost-wrapper\">");

        auto start = page.indexOf(START_TAG);
        if (start == -1)
            throw QString("Failed to find article's begining.");

        auto end = page.indexOf("</div>", start);
        if (end == -1)
            throw QString("Failed to find article's end.");

        article = page.mid(start + START_TAG.size(), end - (start + START_TAG.size()));
    }

    parsePrev(page);
    parseNext(page);
}

QString Page::commentsUrl() const
{
    return "http://" + m_name + ".livejournal.com/" + m_name + "/__rpc_get_thread?journal=" + m_name +
            "&itemid=" + m_number + "&flat=&skip=&media=&expand_all=1&page=" + QString::number(m_commentPage);
}

