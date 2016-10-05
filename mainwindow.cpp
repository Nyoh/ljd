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

#include "downloader.h"
#include "printer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->storageText->setPlainText(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).front());
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

void MainWindow::loadEntry(const QString& number, const QString& name)
{
    Entry* entry = new Entry(*m_contentMgr, ui->storageText->toPlainText(), name, number, this);

    QListWidgetItem* item = new QListWidgetItem(number, ui->entriesList);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    item->setData(Qt::UserRole, QVariant::fromValue<Entry*>(entry));
    item->setSelected(true);
    ui->entriesList->insertItem(ui->entriesList->count(), item);

    connect(entry, SIGNAL(finished()), this, SLOT(showSelectedPage()), Qt::QueuedConnection);
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
            return;
        }
    }

    loadEntry(number, name);
}

const QString MainWindow::configFileName()
{
    const QString& fileName = ui->storageText->toPlainText() + QDir::separator() + ui->nameText->toPlainText() + QDir::separator() + "config.json";
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

    for (size_t i = 0; i != ui->entriesList->count(); i++)
    {
        QListWidgetItem* item = ui->entriesList->item(i);
        Entry* entry = item->data(Qt::UserRole).value<Entry*>();
        entries.push_back(entry->info.number);
    }

    QJsonObject root;
    root["entries"] = entries;

    QJsonDocument saveDoc(root);
    configFile.write(saveDoc.toJson());
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
        loadEntry(entryJson.toString(), ui->nameText->toPlainText());
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
