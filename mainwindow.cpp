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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onRequestFinished()
{
    Printer* printer = new Printer(this);

    ui->viewer->setHtml(printer->print(*m_entry));

    QListWidgetItem* item = new QListWidgetItem("item", ui->entriesList);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
    item->setCheckState(Qt::Unchecked); // AND initialize check state

    ui->entriesList->insertItem(0, item);
}

void MainWindow::on_loadPage_clicked()
{
    QString url = ui->urlText->toPlainText();
    const auto tagStart = url.indexOf(LJ_TAG);
    if (tagStart == -1)
        return;

    const QString& name = url.mid(0, tagStart);
    const auto nextDot = url.indexOf('.', tagStart + LJ_TAG.size());
    if (nextDot == -1)
        return;

    const QString& pageNum = url.mid(tagStart + LJ_TAG.size(), nextDot - (tagStart + LJ_TAG.size()));
    m_entry = new Entry(*m_contentMgr, ".", name, pageNum, this);
    connect( m_entry, SIGNAL(finished()), this, SLOT(onRequestFinished()) );
    m_entry->load();

    //    Image* image = new Image(*netManager, name + "/cavatars", "temp.jpeg", "http://l-userpic.livejournal.com/4456799/793195", this);

}
