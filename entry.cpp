#include "entry.h"

#include <QDir>

#include "contentmanager.h"

Entry::Entry(ContentManager &content, const QString &storage, const QString &name, const QString &number, QObject *parent)
    : QObject(parent)
    , info{storage, name, number}
    , m_content(content)
{
}

void Entry::load()
{
    m_page = m_content.getPage(info.storage + QDir::separator() + info.name + QDir::separator() + "raw_data", info.name, info.number);
    connect(m_page.data(), SIGNAL(finishedAll()), this, SLOT(pageFinished()));
    if (m_page->finished)
        pageFinished();
}

//can triger twice but who cares?
void Entry::pageFinished()
{
    QVector<Comment> commentBundle;
    for (const auto& commentJson : m_page->rawComments)
    {
        commentBundle.push_back(Comment());
        Comment& comment = commentBundle.back();
        comment.name = commentJson["dname"].toString();
        comment.date = commentJson["ctime"].toString();
        comment.text = commentJson["article"].toString();
        comment.id = commentJson["article"].toString();
        comment.text = commentJson["article"].toString();
        comment.text = commentJson["article"].toString();
        comment.text = commentJson["article"].toString();
    }
}
