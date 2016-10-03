#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "contentmanager.h"
#include "image.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_contentMgr = new ContentManager();
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
    m_page = m_contentMgr->getPage(".", name, pageNum).data();
    connect( m_page, SIGNAL(finishedPage(int, bool)), this, SLOT(onRequestFinished(int)) );

//    Image* image = new Image(*netManager, name + "/cavatars", "temp.jpeg", "http://l-userpic.livejournal.com/4456799/793195", this);
}

void MainWindow::onRequestFinished(int)
{
   ui->plainTextEdit_2->setPlainText(m_page->article);
}
