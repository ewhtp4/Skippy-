#include "tcpserver.h"
#include "skippymainwindow.h"
#include "ui_skippymainwindow.h"

using namespace std;

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
    qDebug() <<  this << "created on" << QThread::currentThread();
}

bool TcpServer::listen(const QHostAddress &address, quint16 port)
{
    //Defensive programming - if QtcpServer can not listen return false
    if(!QTcpServer::listen(address,port))
    {
        qCritical() << this << errorString();
        return false;
    }

    //If QTcpServer can listne we creat a thread
    m_thread = new QThread(this);
    m_connections = new TcpRunnable();
    connect(m_thread,&QThread::started,m_connections,&TcpRunnable::start, Qt::QueuedConnection);
    connect(this, &TcpServer::accepting,m_connections,&TcpRunnable::accept, Qt::QueuedConnection);
    connect(this,&TcpServer::finished,m_connections,&TcpRunnable::quit, Qt::QueuedConnection);
    connect(m_connections,&TcpRunnable::finished,this,&TcpServer::complete, Qt::QueuedConnection);

    m_connections->moveToThread(m_thread);
    m_thread->start();

    return true;
}

void TcpServer::close()
{
    qDebug() << this << "closing server";
    m_connections->quit();
    complete();
    emit finished();
    QTcpServer::close();
}

qint64 TcpServer::port()
{
    if(isListening())
    {
        return this->serverPort();
    }
    else
    {
        return 2000; //Defalut Port
    }
}

QStringList TcpServer::getAddresses()
{
    QStringList lst;

    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
        {
            lst.append(address.toString());
        }
    }
    QString publicIP = QString::fromStdString(GetPublicIP());
    lst.append(publicIP);
    return lst;
}

void TcpServer::incomingConnection(qintptr descriptor)
{
    qDebug() << this << "attempting to accept connection" << descriptor;
    TcpConnection *connection = new TcpConnection();
    accept(descriptor, connection);

}

void TcpServer::accept(qintptr descriptor, TcpConnection *connection)
{
    //We dont actually accept the connections in the TcpServer accapt we are forwarding it to TcpRunnable to accept
    qDebug() << this << "accepting the connection" << descriptor;
    connection->moveToThread(m_thread);
    emit accepting(descriptor, connection);
}

void TcpServer::complete()
{
    if(!m_thread)
    {
        qWarning() << this << "exiting complete there was no thread!";
        return;
    }

    qDebug() << this << "Complete called, destroying thread";
    delete m_connections;

    qDebug() << this << "Quitting thread";
    m_thread->quit();
    m_thread->wait();

    delete m_thread;

    qDebug() << this << "complete";
}

string TcpServer::GetPublicIP()
{
    string cmd = "dig +short myip.opendns.com @resolver1.opendns.com";
    string ip = "";
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");
    stream = popen(cmd.c_str(), "r");
    if (stream) {
      while (!feof(stream))
        if (fgets(buffer, max_buffer, stream) != NULL) ip.append(buffer);
      pclose(stream);
    }
    ip.erase(std::remove(ip.begin(), ip.end(), '\n'),ip.end());
    return ip;
}


