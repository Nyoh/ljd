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
}

Printer::Printer(QObject *parent) : QObject(parent)
{
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
        result += "</div></div><br><ul>";

        printComments(comment.children, result);

        result += "</ul></li>";
    }
}

void Printer::printPrevNext(QString &result)
{
    result += "<div style=\"position: relative; margin: 0 30px; padding: 1.5em 0; text-align: center;\">";
    result += "<a href=\"" + contentUrl + "\">Content</a>";

    result += "<ul><li style=\"float: left; list-style: none; font-weight: bold;\"><a href=\"" +
            prevUrl + "\">← " + prevTitle + "</a></li>";

    result += "<li style=\"float: right; list-style: none; font-weight: bold;\"><a href=\"" +
            nextUrl + "\">" + nextTitle + " →</a></li>";
    result += "</ul></div>";
}

QString Printer::print(const Entry &entry, const QString& firstLine, bool toFile)
{
    QString result;
    result.reserve(entry.info.article.size() * 8);

    result += "<div style=\"max-width: 1100px;\">";
    printPrevNext(result);
    result += "<h3>" + firstLine + "</h3>";
    result += "<h3>Url: " + entry.info.url + "</h3>";
    result += "<h3>Date: " + entry.info.date + "</h3>";
    result += entry.info.article;
    result += "</div>";
    printPrevNext(result);

    result += "<br><link rel=\"stylesheet\" href=\"style.css\"><div><ul class=\"tree\">";
    printComments(entry.info.comments, result);
    result += "</ul></div>";
    printPrevNext(result);

    if (!toFile)
        return result;

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

    qDebug() << "Printed: " + entry.info.url;

    return result;
}
