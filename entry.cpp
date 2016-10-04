#include "entry.h"

#include <QDir>
#include <QJsonObject>

#include "downloader.h"

Entry::Entry(Downloader &content, const QString &storage, const QString &name, const QString &number, QObject *parent)
    : QObject(parent)
    , info{storage, name, number}
    , m_content(content)
{
}

void Entry::load()
{
    QDir storage(info.storage);
    storage.mkdir(".");
    storage.mkdir(info.name);
    m_page = new Page(*m_content.getNetwork(), info.storage + QDir::separator() + info.name + QDir::separator() + "raw_data", info.name, info.number);
    connect(m_page, SIGNAL(finished()), this, SLOT(pageFinished()));
    m_page->load();
}

void Entry::pageFinished()
{
    QVector<Comment> commentBundle;
    for (const auto& commentJson : m_page->rawComments)
    {
        commentBundle.push_back(Comment());
        Comment& comment = commentBundle.back();
        QJsonObject rawComment = commentJson.toObject();

        comment.name = rawComment["dname"].toString();
        comment.date = rawComment["ctime"].toString();
        comment.text = rawComment["article"].toString();
        comment.id = rawComment["article"].toString();
        comment.text = rawComment["article"].toString();
        comment.text = rawComment["article"].toString();
        comment.text = rawComment["article"].toString();
    }
}
