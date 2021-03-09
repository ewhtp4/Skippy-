#ifndef PAGEVIEW_H
#define PAGEVIEW_H

#include <QWidget>
#include <QWebEngineView>

#include "ui_pageview.h"

class PageView : public QWidget
{
    Q_OBJECT
public:
    explicit PageView(QWidget *parent = nullptr);
    ~PageView();

    void paintEvent(QPaintEvent *event);
    void loadPage(QString page);
signals:

private:
    Ui::Form *ui;
    QWebEngineView* webView;
};

#endif // PAGEVIEW_H
