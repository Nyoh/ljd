#ifndef ENTRY_H
#define ENTRY_H

#include <QObject>
#include <QSharedPointer>
#include <QQueue>

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
        QString subject;
        QString userpic;
        QString userpicFile;
        QString id;
        QString parent;

        QQueue<Comment> children;
    };

    struct Info
    {
        const QString storage;
        const QString name;
        const QString number;

        QString article;
        QQueue<Comment> comments;
    } info;

    QString print();

signals:
    void finished();

private slots:
    void pageFinished();

private:
    Downloader& m_content;
    Page* m_page;
    void buildTree();
    void printComments(const QQueue<Entry::Comment> &comments, QString &result);
};

#endif // ENTRY_H
