#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QString url = ui->plainTextEdit->toPlainText();
    m_Reply = manager->get( QNetworkRequest( QUrl(url) ));
    connect( m_Reply, SIGNAL(readyRead()), this, SLOT(onRequestFinished()) );
    connect( m_Reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)) );
    connect( m_Reply, SIGNAL(sslErrors(const QList<QSslError>&)), this, SLOT(sslErrors(const QList<QSslError>&)) );


}

void MainWindow::onRequestFinished()
{
if( ! m_Reply )
return;

if( m_Reply->error() == QNetworkReply::NoError )
{
    ui->textEdit->setText(m_Reply->readAll());
}
m_Reply->deleteLater();

}

void MainWindow::error(QNetworkReply::NetworkError code)
{

}

void MainWindow::sslErrors(const QList<QSslError> & errors)
{

}
