#ifndef SKIPPYMAINWINDOW_H
#define SKIPPYMAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QMessageBox>
#include <QtCore>
#include <QtGui>
#include <QInputDialog>
#include <QDirModel>
#include <QString>

#include "httpserver.h"
#include "pageView/pageview.h"
#include "scanner/scanner.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SkippyMainWindow; }
QT_END_NAMESPACE

class SkippyMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    SkippyMainWindow(QWidget *parent = nullptr);
    ~SkippyMainWindow();

    void skippyOutput(QString value);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_makeButton_clicked();
    void on_deleteButton_clicked();
    void on_setButton_clicked();
    void on_listenButton_clicked();
    void on_stopButton_clicked();
    void on_pageButton_clicked();

    void on_scanButton_clicked();

private:
    Ui::SkippyMainWindow *ui;

    void CheckAddressAndPort();
    void CheckForSkippyDir();
    void EmptyDirCheck();
    void CheckDirPremissions();
    void SecurePath();
    void UnsecurePath();
    void FilePremissionCheck();
    void setStarted(bool started);
    void PathSetup();

    //TcpServer m_server;
    HttpServer m_server;
    QDirModel *m_dirModel;
    Scanner scanner;
    QString m_path;
    QString m_defaultPath;
    QString m_startPath;
    QString m_scannerPath;
    PageView pageView;



};
#endif // SKIPPYMAINWINDOW_H
