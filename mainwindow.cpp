#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    m_printer = new Printer(this);;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showSelectedPage()
{
    const auto& selected = ui->entriesList->selectedItems();
    if (selected.isEmpty())
        return;

    const Entry* entry = selected.first()->data(Qt::UserRole).value<Entry*>();
    ui->viewer->setHtml(m_printer->print(*entry));
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

    Entry* entry = new Entry(*m_contentMgr, ".", name, number, this);

    QListWidgetItem* item = new QListWidgetItem(number, ui->entriesList);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    item->setData(Qt::UserRole, QVariant::fromValue<Entry*>(entry));
    item->setSelected(true);
    ui->entriesList->insertItem(ui->entriesList->count(), item);

    connect(entry, SIGNAL(finished()), this, SLOT(showSelectedPage()));
    entry->load();
}
