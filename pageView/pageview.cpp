#include "pageview.h"

PageView::PageView(QWidget *parent)
    : QWidget(parent)
    ,ui(new Ui::Form)
{
    ui->setupUi(this);
    this->setStyleSheet("background-color: rgb(49, 57, 72); color: rgb(139, 148, 190);");
    webView = new QWebEngineView(ui->frame);
}

PageView::~PageView()
{

}

void PageView::paintEvent(QPaintEvent *event)
{
   QWidget::paintEvent(event);
   webView->resize(ui->frame->size());
}

void PageView::loadPage(QString page)
{
     webView->load(QUrl(page));
}



