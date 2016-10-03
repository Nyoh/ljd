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

    const QString& pageNum = url.mid(tagStart + LJ_TAG.size(), nextDot);

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    m_page = new NetPage(*manager, url);
    connect( m_page, SIGNAL(done()), this, SLOT(onRequestFinished()) );
}

void MainWindow::onRequestFinished()
{
    if( m_page->errorMessage.isEmpty())
    {
        ui->plainTextEdit_2->setPlainText(m_page->comments);
    }
}
