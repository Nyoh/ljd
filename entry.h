#ifndef ENTRY_H
#define ENTRY_H

#include <QObject>
#include <QSharedPointer>
#include <QVector>

class ContentManager;
class Page;

class Entry : public QObject
{
    Q_OBJECT
public:
    explicit Entry(ContentManager& content, const QString& storage, const QString& name, const QString& number, QObject *parent = 0);
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
    void updated();
    void finished();

private slots:
    void pageFinished();

private:
    ContentManager& m_content;
    QSharedPointer<Page> m_page;
};

#endif // ENTRY_H
