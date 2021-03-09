#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QDebug>
#include <QTcpServer>
#include <QThread>
#include <QStringList>
#include <QNetworkInterface>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "tcprunnable.h"
#include "tcpconnection.h"

using namespace std;
class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);

    virtual bool listen(const QHostAddress &address, quint16 port);
    virtual void close();
    virtual qint64 port();
    QStringList getAddresses();
    string GetPublicIP();
protected:
    QThread *m_thread;
    TcpRunnable *m_connections;
    //qint64, qHandle, qintptr, uint - depending on the Qt version
    virtual void incomingConnection(qintptr descriptor);
    virtual void accept(qintptr descriptor, TcpConnection *connection);

signals:
    //Create a connection and move it to another thread
    void accepting(qintptr handle, TcpConnection *connection);
    void finished();

public slots:
    void complete();


};

#endif // TCPSERVER_H
