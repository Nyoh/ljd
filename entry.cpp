#include "entry.h"

#include <QDebug>
#include <QDir>
#include <QJsonObject>
#include <QQueue>
#include <QVector>
#include <QCryptographicHash>

#include "downloader.h"

namespace
{
    void filterByName(QQueue<Entry::Comment>& comments, const QString& name)
    {
        QMutableListIterator<Entry::Comment> i(comments);
        while (i.hasNext())
        {
            Entry::Comment& comment = i.next();
            filterByName(comment.children, name);

            if (comment.children.isEmpty() && comment.name != name)
                i.remove();
        }
    }
}

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

void Entry::loadPictures()
{
    for (const auto& image : m_images)
    {
        m_content.get(std::get<0>(image), std::get<1>(image), std::get<2>(image));
    }
}

void Entry::buildTree()
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
        comment.subject = rawComment["subject"].toString();
        comment.userpic = rawComment["userpic"].toString();
        comment.id = QString::number(rawComment["dtalkid"].toInt());
        comment.parent = QString::number(rawComment["parent"].toInt(-1));

        if (info.date.isEmpty())
            info.date = rawComment["ctime"].toString();

        if (comment.userpic.isEmpty())
        {
            continue;
        }

        comment.userpicFile = QString(QCryptographicHash::hash(comment.userpic.toUtf8(), QCryptographicHash::Md5).toHex()) + ".jpeg";
    }

    QHash<QString, QVector<Comment const*>> treeMap;
    for (const auto& comment : commentBundle)
    {
        treeMap[comment.parent].push_back(&comment);
    }

    {
        QQueue<QPair<QString, QQueue<Comment>*>> queue;
        int tasksQueued = 1;
        queue.push_back(QPair<QString, QQueue<Comment>*>("-1", &info.comments));

        while (!queue.isEmpty())
        {
            const auto& pair = queue.front();
            const QVector<Comment const*>& chilrenList = treeMap[pair.first];
            for (const auto& child : chilrenList)
            {
                pair.second->push_back(*child);
                queue.push_back(QPair<QString, QQueue<Comment>*>(child->id, &pair.second->back().children));
                tasksQueued++;
            }
            queue.pop_front();
        }
    }

    QString author = info.name;
    filterByName(info.comments, author.replace('-', '_'));

    QQueue<QQueue<Comment> const*> queue;
    queue.push_back(&info.comments);
    while (!queue.isEmpty())
    {
        const auto& chilrenList = *queue.front();
        for (const auto& child : chilrenList)
        {
            m_images.push_back(std::make_tuple(info.storage + QDir::separator() + info.name + QDir::separator() + "avatars", child.userpic, child.userpicFile));
            queue.push_back(&child.children);
        }
        queue.pop_front();
    }
}

void Entry::processArticle()
{
    info.article = m_page->article;

    static const QString imageTag("<img ");
    static const QString srcTag(" src=\"");
    auto start = info.article.indexOf(imageTag);
    while (start != -1)
    {
        auto srcStart = info.article.indexOf(srcTag, start);
        if (srcStart == -1)
            break;
        srcStart += srcTag.size();

        auto end = info.article.indexOf("\"", srcStart);
        const auto& imageUrl = info.article.mid(srcStart, end - srcStart);
        const auto& imageFileName = QString(QCryptographicHash::hash(imageUrl.toUtf8(), QCryptographicHash::Md5).toHex()) + ".jpeg";
        const auto& imageFilePath = QString("images") + QDir::separator() + imageFileName;

        m_images.push_back(std::make_tuple(info.storage + QDir::separator() + info.name + QDir::separator() + "images",
                                           imageUrl,
                                           imageFileName));

        info.article.replace(srcStart, end - srcStart, imageFilePath);
        start = info.article.indexOf(imageTag, end);
    }
}

void Entry::pageFinished()
{
    buildTree();
    processArticle();

    info.next = m_page->next;
    info.prev = m_page->prev;

    auto start = info.article.indexOf("<h1 ");
    start = info.article.indexOf(">", start);
    start++;
    info.title = info.article.mid(start, info.article.indexOf("</h1>", start) - start);
    info.title = info.title.trimmed();
    info.url = m_page->url;

    qDebug() << "Page loaded: " << info.date << "  " << info.url;
    emit finished();
}
