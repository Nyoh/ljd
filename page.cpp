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
}


Page::Page(const QString& storage, const QString& name, const QString& number, QObject *parent)
    : QObject(parent)
    , m_storage(storage)
    , m_name(name)
    , m_number(number)
{
    qInfo() << "Storage set to " << storageDir.absolutePath();

    if (!loadFirstFromStorage())
        loadFirstFromNet();
}

void Page::netPageLoaded()
{

}

bool Page::loadFirstFromStorage()
{
    QDir storageDir(storage + QDir::separator() + name);
    QFile articleFile(articleFileName(name, number));

    if (storageDir.mkdir(".") || !articleFile.exists())
        return false;

    return true;
}

void Page::loadFirstFromNet()
{
    m_netManager = new QNetworkAccessManager(this);
    m_netPage = new NetPage(*m_netManager, name + LJ_TAG + number + ".html", 1, this);
    connect(m_netPage, SIGNAL(done()), this, SLOT(netPageLoaded()));
    return;
}
