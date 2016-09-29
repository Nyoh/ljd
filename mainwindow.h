#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/qnetworkreply.h>
#include <QPointer>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onRequestFinished();
    void error(QNetworkReply::NetworkError code);
    void sslErrors(const QList<QSslError> & errors);

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QPointer<QNetworkReply> m_Reply;


};

#endif // MAINWINDOW_H
