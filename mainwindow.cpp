#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QNetworkAccessManager>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString url = ui->plainTextEdit->toPlainText();
    const auto tagStart = url.indexOf(LJ_TAG);
    if (tagStart == -1)
        return;

    const QString& name = url.mid(0, tagStart);
    const auto nextDot = url.indexOf('.', tagStart + LJ_TAG.size());
    if (nextDot == -1)
        return;

    const QString& pageNum = url.mid(tagStart + LJ_TAG.size(), nextDot - (tagStart + LJ_TAG.size()));

    m_page = new Page(".", name, pageNum, this);
    connect( m_page, SIGNAL(finished(int)), this, SLOT(onRequestFinished()) );
}

void MainWindow::onRequestFinished()
{
   ui->plainTextEdit_2->setPlainText(m_page->info.article);
}
