#ifndef PRINTER_H
#define PRINTER_H

#include <QObject>

#include "entry.h"

class Printer : public QObject
{
    Q_OBJECT
public:
    explicit Printer(QObject *parent = 0);

    QString print(const Entry& entry, const QString& firstLine, bool toFile = false);

    QString prevUrl;
    QString prevTitle;
    QString nextUrl;
    QString nextTitle;
    QString contentUrl;

private:
    void printComments(const QQueue<Entry::Comment> &comments, QString &result);
    void printPrevNext(QString &result);
};

#endif // PRINTER_H
