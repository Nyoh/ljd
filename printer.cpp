#include "printer.h"

#include <QtDebug>
#include <QDir>
#include <QMutableListIterator>
#include <QTextStream>

namespace
{
    QString getFileName(const QString& storage, const QString& name, const QString& number)
    {
        return storage + QDir::separator() + name + QDir::separator() + number + ".html";
    }

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

Printer::Printer(QObject *parent) : QObject(parent)
{

}

QQueue<Entry::Comment> Printer::filterComments(const QQueue<Entry::Comment>& comments, const QString &author)
{
    if (!this->comments)
        return QQueue<Entry::Comment>();

    if (!filterCommentsByAuthor)
        return comments;

    QQueue<Entry::Comment> result = comments;
    filterByName(result, author);
    return result;
}

void Printer::printComments(const QQueue<Entry::Comment>& comments, QString &result)
{
    for (const auto& comment : comments)
    {
        result += "<li><div id=\"main\">";

        result += "<img src=\"avatars//" + comment.userpicFile + "\" width=\"50\" height=\"50\" align=\"left\">";
        result +=  "<b>" + comment.name + "</b> (" + comment.date + "): ";
        result += "<div>";
        result += comment.text;
        result += "</div></div><ul>";

        printComments(comment.children, result);

        result += "</ul></li>";
    }
}

QString Printer::print(const Entry &entry)
{
    QString result = entry.info.article;
    result += "<br><link rel=\"stylesheet\" href=\"style.css\"><div><ul class=\"tree\">";
    printComments(filterComments(entry.info.comments, entry.info.name), result);
    result += "</ul></div>";

    QFile file(getFileName(entry.info.storage, entry.info.name, entry.info.number));
    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << "Failed to write the file " + getFileName(entry.info.storage, entry.info.name, entry.info.number);
        return result;
    }
    QTextStream stream(&file);
    stream.setGenerateByteOrderMark(true);
    stream.setCodec("UTF-8");
    stream << result.toUtf8();

    return result;
}
