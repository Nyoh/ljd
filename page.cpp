#include "page.h"
#include "netpage.h"

#include <QtDebug>
#include <QDir>
#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QJsonDocument>

namespace
{
    QString articleFileName(const QString& name, const QString& number)
    {
        return name + QDir::separator() + number + "_article.html";
    }

    QString url(const QString& name, const QString& number)
    {
        return "http://" + name + LJ_TAG + number + ".html";
    }
}


Page::Page(const QString& storage, const QString& name, const QString& number, QObject *parent)
    : QObject(parent)
    , m_storage(storage)
    , m_name(name)
    , m_number(number)
{
}

void Page::load()
{
    if (!loadFirstFromStorage())
    {
        m_netManager = new QNetworkAccessManager(this);
        m_netPage = new NetPage(*m_netManager, url(m_name, m_number), 1, this);
        connect(m_netPage, SIGNAL(done()), this, SLOT(loadedFromNet()));
    }
}

bool Page::loadFirstFromStorage()
{
    QDir storageDir(m_storage + QDir::separator() + m_name);
    qInfo() << "Storage set to " << storageDir.absolutePath();

    if (storageDir.mkdir("."))
        return false;

    {
        QFile partsFile(articleFileName(m_name, m_number) + ".json");
        if (!partsFile.open(QIODevice::ReadOnly))
        {
            qCritical() << "Failed to read the file " + articleFileName(m_name, m_number) + ".json";
            return false;
        }

        QJsonParseError error;
        QJsonObject root = QJsonDocument::fromJson(partsFile.readAll(), &error).object();
        QJsonArray commentsArray = root["raw_comments"].toArray();
        if (error.error != QJsonParseError::NoError)
        {
            qCritical() << "Failed to parse the file " + articleFileName(m_name, m_number) + ".json";
            return false;
        }

        for (int i = 0; i < commentsArray.size(); i++)
        {
            QJsonObject commentObject = commentsArray[i].toObject();
            info.rawComments.push_back(commentObject);
        }
        m_commentPagesLoaded = root["pages_loaded"].toInt(1);
    }

    {
        QFile articleFile(articleFileName(m_name, m_number));
        if (!articleFile.open(QIODevice::ReadOnly))
        {
            qCritical() << "Failed to read the file " + articleFileName(m_name, m_number);
            return false;
        }

        QTextStream stream(&articleFile);
        stream.setGenerateByteOrderMark(true);
        stream.setCodec("UTF-8");
        info.article = stream.readAll();
    }

    emit finished(m_commentPagesLoaded);
    return true;
}

void Page::loadedFromNet()
{
    if (!m_netPage->errorMessage.isEmpty())
    {
        qWarning() << "Failed to load " + url(m_name, m_number) + ". " + m_netPage->errorMessage;
        //delete m_netPage;
        //m_netPage = new NetPage(*m_netManager, url(m_name, m_number), 1, this);
        connect(m_netPage, SIGNAL(done()), this, SLOT(loadedFromNet()));
        return;
    }
    qDebug() << "Page loaded: " + url(m_name, m_number);

    m_commentPagesLoaded++;
    info.article = m_netPage->article;
    QJsonDocument pageJson = QJsonDocument::fromJson(m_netPage->comments.toUtf8());
    QJsonArray newComments = pageJson.object()["comments"].toArray();

    for (int i = 0; i < newComments.size(); i++)
    {
        QJsonObject commentObject = newComments[i].toObject();
        info.rawComments.push_back(commentObject);
    }

    save();
    emit finished(m_commentPagesLoaded);

    if (!m_netPage->lastCommentPage)
    {
        //delete m_netPage;
        m_netPage = new NetPage(*m_netManager, url(m_name, m_number), m_commentPagesLoaded + 1, this);
        connect(m_netPage, SIGNAL(done()), this, SLOT(loadedFromNet()));
    }
}

void Page::save()
{
    {
        QFile articleFile(articleFileName(m_name, m_number));
        if (!articleFile.open(QIODevice::WriteOnly))
        {
            qCritical() << "Failed to write to file " + articleFileName(m_name, m_number);
            return;
        }

        QTextStream stream(&articleFile);
        stream.setGenerateByteOrderMark(true);
        stream.setCodec("UTF-8");
        stream << info.article.toUtf8();
    }

    {
        QFile partsFile(articleFileName(m_name, m_number) + ".json");
        if (!partsFile.open(QIODevice::WriteOnly))
        {
            qCritical() << "Failed to write to file " + articleFileName(m_name, m_number) + ".json";
            return;
        }

        QJsonArray commentsJson;
        for (const auto& comment : info.rawComments)
            commentsJson.append(comment);

        QJsonObject root;
        root["raw_comments"] = commentsJson;
        root["pages_loaded"] = m_commentPagesLoaded;

        QJsonDocument saveDoc(root);
        partsFile.write(saveDoc.toJson());
    }
}
