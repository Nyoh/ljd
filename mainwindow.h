#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "page.h"

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
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    Page* m_page;


};

#endif // MAINWINDOW_H
