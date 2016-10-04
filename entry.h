#ifndef ENTRY_H
#define ENTRY_H

#include <QObject>
#include <QSharedPointer>
#include <QVector>

class Downloader;
class Page;

class Entry : public QObject
{
    Q_OBJECT
public:
    explicit Entry(Downloader& content, const QString& storage, const QString& name, const QString& number, QObject *parent = 0);
    void load();

    struct Comment
    {
        QString name;
        QString text;
        QString date;
        QString avatar;
        QString id;
        QString parent;

        QVector<Comment> children;
    };

    struct Info
    {
        const QString storage;
        const QString name;
        const QString number;

        QVector<Comment> comments;
    } info;

signals:
    void finished();

private slots:
    void pageFinished();

private:
    Downloader& m_content;
    Page* m_page;
};

#endif // ENTRY_H
