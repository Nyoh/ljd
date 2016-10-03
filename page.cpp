#include "page.h"
#include "netpage.h"

#include <QtDebug>
#include <QDir>
#include <QNetworkAccessManager>

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
    if (!loadFirstFromStorage())
    {
        m_netManager = new QNetworkAccessManager(this);
        loadFirstFromNet();
    }
}

void Page::netPageLoaded()
{

}

bool Page::loadFirstFromStorage()
{
    QDir storageDir(m_storage + QDir::separator() + m_name);
    qInfo() << "Storage set to " << storageDir.absolutePath();

    QFile articleFile(articleFileName(m_name, m_number));

    if (storageDir.mkdir(".") || !articleFile.exists())
        return false;

    return false;
}

void Page::loadFirstFromNet()
{
    m_netPage = new NetPage(*m_netManager, url(m_name, m_number), 1, this);
    connect(m_netPage, SIGNAL(done()), this, SLOT(firstFromNetLoaded()));
}

void Page::firstFromNetLoaded()
{
    if (!m_netPage->errorMessage.isEmpty())
    {
        qWarning() << "Failed to load " + url(m_name, m_number) + ". " + m_netPage->errorMessage;
        //delete m_netPage;
        //m_netPage = new NetPage(*m_netManager, url(m_name, m_number), 1, this);
        connect(m_netPage, SIGNAL(done()), this, SLOT(firstFromNetLoaded()));
        return;
    }

    info.article = m_netPage->article;

    save();
    emit finished(1);
}

void Page::save()
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
