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

    QString prevUrl;
    QString prevTitle;
    QString nextUrl;
    QString nextTitle;
    QString subtitle;

private:
    void printComments(const QQueue<Entry::Comment> &comments, QString &result);
    QQueue<Entry::Comment> filterComments(const QQueue<Entry::Comment> &comments, const QString& author);
};

#endif // PRINTER_H
