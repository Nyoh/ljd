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
    void showSelectedPage();

private slots:
    void on_loadPage_clicked();

    void on_saveConfig_clicked();

    void on_loadConfig_clicked();

    void on_nextButton_clicked();

    void on_prevButton_clicked();

private:
    Ui::MainWindow *ui;
    Downloader* m_contentMgr;
    const QString configFileName();
    size_t m_skipBeforeUpdate = 0;
    void loadEntry(const QString& number, const QString& name);
    void go(bool next);
};

#endif // MAINWINDOW_H
