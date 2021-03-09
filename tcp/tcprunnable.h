#ifndef TCPRUNNABLE_H
#define TCPRUNNABLE_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QTcpSocket>
#include <QMap>
#include <QReadWriteLock>
#include "tcpconnection.h"

class TcpRunnable : public QObject
{
    Q_OBJECT
public:
    explicit TcpRunnable(QObject *parent = nullptr);
    virtual int count();

protected:
    QMap<QTcpSocket*, TcpConnection*> m_connections;
    void removeSocket(QTcpSocket *socket);

signals:
    void quitting();
    void finished();

protected slots:
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);

public slots:
    void start();
    void quit();
    void accept(qintptr handle, TcpConnection *connection);
};

#endif // TCPRUNNABLE_H
