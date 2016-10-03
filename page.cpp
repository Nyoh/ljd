#include "page.h"
#include "netpage.h"

#include <QtDebug>
#include <QDir>
#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QJsonDocument>

namespace
{
    QString articleFileName(const QString& storage)
    {
        return storage + QDir::separator() + "article.html";
    }

    QString url(const QString& name, const QString& number)
    {
        return "http://" + name + LJ_TAG + number + ".html";
    }
}


Page::Page(QNetworkAccessManager& netManager, const QString& storage, const QString& name, const QString& number, QObject *parent)
    : QObject(parent)
    , m_storage(storage)
    , m_name(name)
    , m_number(number)
    , m_netManager(netManager)
{
}

void Page::load()
{
    started = true;
    if (!loadFirstFromStorage())
    {
        m_netPage = new NetPage(m_netManager, url(m_name, m_number), m_commentPagesLoaded + 1, this);
        connect(m_netPage, SIGNAL(done()), this, SLOT(loadedFromNet()));
        m_netPage->load();
    }
}

bool Page::loadFirstFromStorage()
{
    QDir storageDir(m_storage + QDir::separator() + m_name);
    qInfo() << "Storage set to " << storageDir.absolutePath();

    if (storageDir.mkdir("."))
        return false;

    {
        QFile partsFile(articleFileName(m_storage) + ".json");
        if (!partsFile.open(QIODevice::ReadOnly))
        {
            qCritical() << "Failed to read the file " + articleFileName(m_storage) + ".json";
            return false;
        }

        QJsonParseError error;
        QJsonObject root = QJsonDocument::fromJson(partsFile.readAll(), &error).object();
        QJsonArray commentsArray = root["raw_comments"].toArray();
        if (error.error != QJsonParseError::NoError)
        {
            qCritical() << "Failed to parse the file " + articleFileName(m_storage) + ".json";
            return false;
        }

        for (int i = 0; i < commentsArray.size(); i++)
        {
            QJsonObject commentObject = commentsArray[i].toObject();
            rawComments.push_back(commentObject);
        }
        m_commentPagesLoaded = root["pages_loaded"].toInt(1);
        m_commentsFinished = root["comments_finished"].toBool(false);
    }

    {
        QFile articleFile(articleFileName(m_storage));
        if (!articleFile.open(QIODevice::ReadOnly))
        {
            qCritical() << "Failed to read the file " + articleFileName(m_storage);
            return false;
        }

        QTextStream stream(&articleFile);
        stream.setGenerateByteOrderMark(true);
        stream.setCodec("UTF-8");
        article = stream.readAll();
    }

    emit finishedPage(m_commentPagesLoaded, false);
    return m_commentsFinished;
}

void Page::loadedFromNet()
{
    if (!m_netPage->errorMessage.isEmpty())
    {
        qWarning() << "Failed to load " + url(m_name, m_number) + ". " + m_netPage->errorMessage;
        m_netPage->deleteLater();
        emit finishedPage(m_commentPagesLoaded, true);
        return;
    }
    qDebug() << "Page loaded: " + url(m_name, m_number);

    m_commentPagesLoaded++;
    article = m_netPage->article;
    QJsonDocument pageJson = QJsonDocument::fromJson(m_netPage->comments.toUtf8());
    QJsonArray newComments = pageJson.object()["comments"].toArray();

    for (int i = 0; i < newComments.size(); i++)
    {
        QJsonObject commentObject = newComments[i].toObject();
        rawComments.push_back(commentObject);
    }

    m_commentsFinished = m_netPage->lastCommentPage;

    save();
    emit finishedPage(m_commentPagesLoaded, false);

    if (!m_netPage->lastCommentPage && !finished)
    {
        m_netPage->deleteLater();
        m_netPage = new NetPage(m_netManager, url(m_name, m_number), m_commentPagesLoaded + 1, this);
        connect(m_netPage, SIGNAL(done()), this, SLOT(loadedFromNet()));
        m_netPage->load();
    }
    else
    {
        finished = true;
        emit finishedAll();
    }
}

void Page::save()
{
    {
        QFile articleFile(articleFileName(m_storage));
        if (!articleFile.open(QIODevice::WriteOnly))
        {
            qCritical() << "Failed to write to file " + articleFileName(m_storage);
            return;
        }

        QTextStream stream(&articleFile);
        stream.setGenerateByteOrderMark(true);
        stream.setCodec("UTF-8");
        stream << article.toUtf8();
    }

    {
        QFile partsFile(articleFileName(m_storage) + ".json");
        if (!partsFile.open(QIODevice::WriteOnly))
        {
            qCritical() << "Failed to write to file " + articleFileName(m_storage) + ".json";
            return;
        }

        QJsonArray commentsJson;
        for (const auto& comment : rawComments)
            commentsJson.append(comment);

        QJsonObject root;
        root["raw_comments"] = commentsJson;
        root["pages_loaded"] = m_commentPagesLoaded;
        root["comments_finished"] = m_commentsFinished;

        QJsonDocument saveDoc(root);
        partsFile.write(saveDoc.toJson());
    }
}
