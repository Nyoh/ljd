#ifndef ENTRY_H
#define ENTRY_H

#include <tuple>

#include <QObject>
#include <QSharedPointer>
#include <QQueue>
#include <QVector>

class Downloader;
class Page;

class Entry : public QObject
{
    Q_OBJECT
public:
    explicit Entry(Downloader& content, const QString& storage, const QString& name, const QString& number, QObject *parent = 0);
    void load();
    void loadPictures();

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
        bool visible = false;

        QQueue<Comment> children;
    };

    struct Info
    {
        const QString storage;
        const QString name;
        const QString number;

        QString article;
        QString title;
        QString prev;
        QString next;
        QString url;
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
    QVector<std::tuple<QString, QString, QString>> m_images;
    void buildTree();
    void printComments(const QQueue<Entry::Comment> &comments, QString &result);
    void processArticle();
};

Q_DECLARE_METATYPE(Entry*)

#endif // ENTRY_H
