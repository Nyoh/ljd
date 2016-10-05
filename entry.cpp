#include "entry.h"

#include <QDir>
#include <QJsonObject>
#include <QQueue>
#include <QVector>
#include <QCryptographicHash>

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

        if (comment.userpic.isEmpty())
        {
            continue;
        }

        comment.userpicFile = QString(QCryptographicHash::hash(comment.userpic.toUtf8(), QCryptographicHash::Md5).toHex()) + ".jpeg";

        m_images.push_back(std::make_tuple(info.storage + QDir::separator() + info.name + QDir::separator() + "avatars",
                                           comment.userpic,
                                           comment.userpicFile));
    }

    QHash<QString, QVector<Comment const*>> treeMap;
    for (const auto& comment : commentBundle)
    {
        treeMap[comment.parent].push_back(&comment);
    }
    QHash<QString, int> used;
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
            used[child->id] = 1;
            tasksQueued++;
        }
        queue.pop_front();
    }
}

void Entry::processArticle()
{
    info.article = m_page->article;

    static const QString imageTag("<img src=\"");
    auto start = info.article.indexOf(imageTag);
    while (start != -1)
    {
        start += imageTag.size();
        auto end = info.article.indexOf("\"", start);
        const auto& imageUrl = info.article.mid(start, end - start);
        const auto& imageFileName = QString(QCryptographicHash::hash(imageUrl.toUtf8(), QCryptographicHash::Md5).toHex()) + ".jpeg";
        const auto& imageFilePath = QString("images") + QDir::separator() + imageFileName;

        m_images.push_back(std::make_tuple(info.storage + QDir::separator() + info.name + QDir::separator() + "images",
                                           imageUrl,
                                           imageFileName));

        info.article.replace(start, end - start, imageFilePath);
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

    emit finished();
}
