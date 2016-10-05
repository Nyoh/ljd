#ifndef PRINTER_H
#define PRINTER_H

#include <QObject>

#include "entry.h"

class Printer : public QObject
{
    Q_OBJECT
public:
    explicit Printer(QObject *parent = 0);

    QString print(const Entry& entry);

    bool comments = true;
    bool filterCommentsByAuthor = true;

    QString prevUrl = "dfsdg";
    QString prevTitle = "prev";
    QString nextUrl = "adfsd";
    QString nextTitle = "next";
    QString contentUrl = "content";

private:
    void printComments(const QQueue<Entry::Comment> &comments, QString &result);
    QQueue<Entry::Comment> filterComments(const QQueue<Entry::Comment> &comments, const QString& author);
    void printPrevNext(QString &result);
};

#endif // PRINTER_H
