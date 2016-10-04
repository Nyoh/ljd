#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "entry.h"

class Downloader;

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

private slots:
    void on_loadPage_clicked();

private:
    Ui::MainWindow *ui;
    Entry* m_entry;
    Downloader* m_contentMgr;
};

#endif // MAINWINDOW_H
