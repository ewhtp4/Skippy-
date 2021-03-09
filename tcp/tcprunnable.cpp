#include "tcprunnable.h"

TcpRunnable::TcpRunnable(QObject *parent) : QObject(parent)
{
    qDebug() << this << "created";
}

int TcpRunnable::count()
{
    //Locks the int value to the thread we are currently on to prevent thread errors
    QReadWriteLock lock;
    lock.lockForRead();
    int value = m_connections.count();
    //Once done unlock
    lock.unlock();

    return value;
}

void TcpRunnable::removeSocket(QTcpSocket *socket)
{
    //Defensive programming
    if(!socket) return;
    //Speed Defensive programming
    if(!m_connections.contains(socket)) return;

    qDebug() << this << "removing socket = " <<  socket;

    //Need ro close the socker before we get read of it
    if(socket->isOpen())
    {
        qDebug() << this << "socket is open, attempting to close it " << socket;
        //We need to disconnect all the signals and slots out of the socket object
        //to avoid creating a signal collision
        socket->disconnect();
        socket->close();
    }

    qDebug() << this << "deleting socket" << socket;
    m_connections.remove(socket);
    //Using deleteLater in case socket is still in use, it will in that case crash the
    //program and create a segment fault
    socket->deleteLater();

    qDebug() << this << "client count = " << m_connections.count();

}

void TcpRunnable::disconnected()
{
    if(!sender()) return;
    qDebug() << this << "disconnecting socket"<< sender();

    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    if(!socket) return;

    removeSocket(socket);
}

void TcpRunnable::error(QAbstractSocket::SocketError socketError)
{
    if(!sender()) return;
    qDebug() << this << "error in socket" << sender() << " error = " << socketError;

    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    if(!socket) return;

    removeSocket(socket);
}

void TcpRunnable::start()
{
    qDebug() << this << "connections started on" << QThread::currentThread();
}

void TcpRunnable::quit()
{
    if(!sender()) return;
    qDebug() << this << "connections quitting";

    foreach(QTcpSocket *socket, m_connections.keys())
    {
        qDebug() << this << "closing socket" << socket;
        removeSocket(socket);
    }
    qDebug() << this << "finishing";
    emit finished();
}

void TcpRunnable::accept(qintptr handle, TcpConnection *connection)
{
    QTcpSocket *socket = new QTcpSocket(this);

    if(!socket->setSocketDescriptor(handle))
    {
        qWarning() << this << "could not accept connection" << handle;
        connection->deleteLater();
        return;
    }
    connect(socket,&QTcpSocket::disconnected,this,&TcpRunnable::disconnected);
    connect(socket,static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),this,&TcpRunnable::error);

    connection->moveToThread(QThread::currentThread());
    connection->setSocket(socket);
    m_connections.insert(socket, connection);

    qDebug() << this << "clients = " << m_connections.count();
    emit socket->connected();
}
