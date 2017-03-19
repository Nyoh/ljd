#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "entry.h"
#include "calendar.h"

#include <memory>

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

    void on_printBook_clicked();

    void on_loadPicsButton_clicked();

    void on_loadCalendar_clicked();
    void loadIfNew(const QString& number, const QString& name);

private:
    Ui::MainWindow *ui;
    Downloader* m_contentMgr;
    const QString configFileName();
    size_t m_skipBeforeUpdate = 0;
    std::unique_ptr<Calendar> m_calendar;

    void go(bool next);
    void loadEntry(const QString& number, const QString& name, bool checked);
};

#endif // MAINWINDOW_H
