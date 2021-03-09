#include "httpserver.h"

HttpServer::HttpServer(QObject *parent) : TcpServer(parent)
{
    /*Default root path is set to the application path
     *big security issue, never to be used in production code
     *TODO implement the user setting path
     */
    m_root = QCoreApplication::applicationDirPath();
    //Default rate 15kb/s
    m_rate = 15360;
}

QString HttpServer::root()
{
    return m_root;
}

void HttpServer::setRoot(QString path)
{
    //TODO implement the user setting path
    //Defensive programming
    m_root = path;
    //Removing trailing backslash
    if(m_root.endsWith("/") || m_root.endsWith("\\"))
    {
        //m_root = m_root without the last character
        m_root = m_root.mid(0, m_root.length() - 1);
    }
    qDebug() << this << "root set to:" << m_root;
}

int HttpServer::rate()
{
    return m_rate;
}

void HttpServer::setRate(int value)
{
    m_rate = value;
    qDebug() << this << "rate set to:" << value;
}

void HttpServer::incomingConnection(qintptr descriptor)
{
    qDebug() << this << "Incomming HTTP connection: " << descriptor;

    HttpConnection *connection = new HttpConnection();
    //TO DO = set rate and root
    connection->setRate(m_rate);
    connection->setRoot(m_root);
    accept(descriptor,connection);
}
