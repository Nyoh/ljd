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

#include "downloader.h"
#include "printer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_contentMgr = new Downloader();
    m_printer = new Printer(this);

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

    const Entry* entry = selected.first()->data(Qt::UserRole).value<Entry*>();
    ui->viewer->setHtml(m_printer->print(*entry));
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
