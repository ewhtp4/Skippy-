#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QMap>
#include <QCoreApplication>
#include <QStringList>
#include "tcpconnection.h"
#include "ratetransfer.h"

class HttpConnection : public TcpConnection
{
    Q_OBJECT
public:
    explicit HttpConnection(QObject *parent = nullptr);

    QString root();
    void setRoot(QString path);
    int rate();
    void setRate(int value);

protected:
    //We are essentially making a file server
    QFile *m_file;
    QString m_root;
    int m_rate;

    /*HTTP protocol
     * 1. reseve the request,(m_request)
     * 2.send the information(metadata) describing the response(m_response)
     * 3.Send the actual file(m_mime - mime type)
     */
    QMap<QString, QString> m_request;
    QMap<QString, QString> m_response;
    QMap<QString, QString> m_mime;
    RateTransfer *m_transfer;

    void processGet(QByteArray data);
    void handleRequest();
    void sendFile(QString file);
    QString getMimeType(QString path);

signals:

public slots:
    virtual void connected();
    virtual void disconnected();
    virtual void readyRead();
    virtual void bytesWritten(qint64 bytes);
    virtual void stateChanged(QAbstractSocket::SocketState socketState);
    virtual void error(QAbstractSocket::SocketError socketError);

    void started();
    void transfered(qint64 bytes);//qint64 - integer big enough to hold a size of a file
    void finished();
    void transferError();

};

#endif // HTTPCONNECTION_H
