#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "entry.h"
#include "printer.h"

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
    void showSelectedPage();

private slots:
    void on_loadPage_clicked();

private:
    Ui::MainWindow *ui;
    Downloader* m_contentMgr;
    Printer* m_printer;
};

#endif // MAINWINDOW_H
