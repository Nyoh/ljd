#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTextDocument>
#include <QTextFrame>
#include <QStandardPaths>
#include <QScrollBar>

#include "downloader.h"
#include "printer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
 //   ui->storageText->setPlainText(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).front());
    m_contentMgr = new Downloader();

    connect(ui->entriesList, SIGNAL(currentRowChanged(int)), this, SLOT(showSelectedPage()), Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showSelectedPage()
{
    if (m_skipBeforeUpdate > 0)
    {
        m_skipBeforeUpdate--;
        return;
    }

    const auto& selected = ui->entriesList->selectedItems();
    if (selected.isEmpty())
        return;

    const int index = ui->entriesList->row(selected.first());
    const Entry* entry = selected.first()->data(Qt::UserRole).value<Entry*>();

    Printer printer;
    if (index != 0)
    {
        Entry* prev = ui->entriesList->item(index - 1)->data(Qt::UserRole).value<Entry*>();
        printer.prevTitle = prev->info.title;
        printer.prevUrl = prev->info.number + ".html";
    }
    if (index + 1 < ui->entriesList->count())
    {
        Entry* next = ui->entriesList->item(index + 1)->data(Qt::UserRole).value<Entry*>();
        printer.nextTitle = next->info.title;
        printer.nextUrl = next->info.number + ".html";
    }

    ui->numberText->setPlainText(entry->info.number);
    ui->viewer->setHtml(printer.print(*entry));
}

void MainWindow::loadEntry(const QString& number, const QString& name, bool checked)
{
    Entry* entry = new Entry(*m_contentMgr, ui->storageText->toPlainText(), name, number, this);

    QListWidgetItem* item = new QListWidgetItem(number, ui->entriesList);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    item->setData(Qt::UserRole, QVariant::fromValue<Entry*>(entry));
    item->setSelected(true);
    ui->entriesList->insertItem(ui->entriesList->count(), item);
    ui->entriesList->verticalScrollBar()->setValue(ui->entriesList->verticalScrollBar()->maximum());

    connect(entry, SIGNAL(finished()), this, SLOT(showSelectedPage()), Qt::QueuedConnection);
    connect(entry, &Entry::finished, [item, entry](){item->setText(entry->info.number + " - " + entry->info.title);});
    entry->load();
}

void MainWindow::on_loadPage_clicked()
{
    const QString& name = ui->nameText->toPlainText();
    const QString& number = ui->numberText->toPlainText();

    for (size_t i = 0; i != ui->entriesList->count(); i++)
    {
        QListWidgetItem* item = ui->entriesList->item(i);
        Entry* entry = item->data(Qt::UserRole).value<Entry*>();
        if (entry->info.name == name && entry->info.number == number)
        {
            item->setSelected(true);
            showSelectedPage();
            return;
        }
    }

    loadEntry(number, name, true);
}

const QString MainWindow::configFileName()
{
    const QString& fileName = ui->storageText->toPlainText() + QDir::separator() + ui->nameText->toPlainText() + QDir::separator() + "!config.json";
    return fileName;
}

void MainWindow::on_saveConfig_clicked()
{
    const QString& fileName = configFileName();
    QFile configFile(fileName);
    if (!configFile.open(QIODevice::WriteOnly))
    {
        qCritical() << "Failed to save config file " + fileName;
        return;
    }

    QJsonArray entries;

    QVector<QPair<Entry*, bool>> entriesVector;
    for (size_t i = 0; i != ui->entriesList->count(); i++)
    {
        QListWidgetItem* item = ui->entriesList->item(i);
        entriesVector.push_back(QPair<Entry*, bool>(item->data(Qt::UserRole).value<Entry*>(), item->checkState() == Qt::Checked));
    }

    qSort(entriesVector.begin(), entriesVector.end(), [](const QPair<Entry*, bool>& left, const QPair<Entry*, bool>& right){
        const auto& l = left.first->info.number;
        const auto& r = right.first->info.number;
        if (l.size() < r.size())
            return true;
        else if (l.size() > r.size())
            return false;

        return l < r;
    });

    for (const auto& pair : entriesVector)
    {
        QJsonObject entryJson;
        entryJson["number"] = pair.first->info.number;
        entryJson["checked"] = pair.second;

        entries.push_back(entryJson);
    }

    QJsonObject root;
    root["entries"] = entries;

    QJsonDocument saveDoc(root);
    configFile.write(saveDoc.toJson());

    qDebug() << "Config saved.";
}

void MainWindow::on_loadConfig_clicked()
{
    const QString& fileName = configFileName();
    QFile configFile(fileName);
    if (!configFile.open(QIODevice::ReadOnly))
    {
        qCritical() << "Failed to read config file " + fileName;
        return;
    }

    QJsonParseError error;
    QJsonObject root = QJsonDocument::fromJson(configFile.readAll(), &error).object();
    if (error.error != QJsonParseError::NoError)
    {
        qCritical() << "Failed to parse config file " + fileName;
        return;
    }

    ui->entriesList->clear();
    QJsonArray entries = root["entries"].toArray();
    for (const auto& entryJson : entries)
    {
        auto entry = entryJson.toObject();
        loadEntry(entry["number"].toString(), ui->nameText->toPlainText(), entry["checked"].toBool());
        m_skipBeforeUpdate++;
    }
    m_skipBeforeUpdate--;
}

void MainWindow::go(bool next)
{
    const auto& selected = ui->entriesList->selectedItems();
    if (selected.isEmpty())
        return;

    const int index = ui->entriesList->row(selected.first());
    const Entry* entry = selected.first()->data(Qt::UserRole).value<Entry*>();

    const QString& url = next ? entry->info.next : entry->info.prev;
    if (url.isEmpty())
        return;

    const auto tagStart = url.indexOf(LJ_TAG);
    if (tagStart == -1)
        return;

    const auto nextDot = url.indexOf('.', tagStart + LJ_TAG.size());
    if (nextDot == -1)
        return;

    const QString& pageNum = url.mid(tagStart + LJ_TAG.size(), nextDot - (tagStart + LJ_TAG.size()));
    ui->numberText->setPlainText(pageNum);
    on_loadPage_clicked();
}

void MainWindow::on_nextButton_clicked()
{
    go(true);
}

void MainWindow::on_prevButton_clicked()
{
    go(false);
}

void MainWindow::on_printBook_clicked()
{
    QVector<Entry*> entries;
    for (size_t i = 0; i != ui->entriesList->count(); i++)
    {
        QListWidgetItem* item = ui->entriesList->item(i);
        if (item->checkState() != Qt::Checked)
            continue;

        entries.push_back(item->data(Qt::UserRole).value<Entry*>());
    }

    for (size_t i = 0; i != entries.size(); i++)
    {
        Entry* entry = entries[i];

        Printer printer;
        if (i != 0)
        {
            Entry* prev = entries[i - 1];
            printer.prevTitle = prev->info.title;
            printer.prevUrl = prev->info.number + ".html";
        }
        if (i + 1 != entries.size())
        {
            Entry* next = entries[i + 1];
            printer.nextTitle = next->info.title;
            printer.nextUrl = next->info.number + ".html";
        }
        printer.contentUrl = "!content.html";

        printer.print(*entry, true);
    }


    QFile file(ui->storageText->toPlainText() + QDir::separator() + ui->nameText->toPlainText() + QDir::separator() + "!content.html");
    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << "Failed to write the file " + ui->storageText->toPlainText() + QDir::separator() + ui->nameText->toPlainText() + QDir::separator() + "!content.html";
        return;
    }

    QString result;
    for (const auto& entry : entries)
    {
        result += "<div><a href=" + entry->info.number + ".html>" + entry->info.title + "</a></div>";
    }

    QTextStream stream(&file);
    stream.setGenerateByteOrderMark(true);
    stream.setCodec("UTF-8");
    stream << result.toUtf8();

    qDebug() << "Printed: content.";
}

void MainWindow::on_loadPicsButton_clicked()
{
    QVector<Entry*> entries;
    for (size_t i = 0; i != ui->entriesList->count(); i++)
    {
        QListWidgetItem* item = ui->entriesList->item(i);
        if (item->checkState() != Qt::Checked)
            continue;

        item->data(Qt::UserRole).value<Entry*>()->loadPictures();

    }
}
