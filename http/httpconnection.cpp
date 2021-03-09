#include "httpconnection.h"

HttpConnection::HttpConnection(QObject *parent) : TcpConnection(parent)
{
    qDebug() << this << "Created";

    //Setup mime types
    m_mime.insert("txt","text/plain");
    m_mime.insert("htm","text/html");
    m_mime.insert("html","text/html");
    m_mime.insert("xml","text/xml");
    m_mime.insert("js","text/javascript");
    m_mime.insert("rtf","text/rtf");
    m_mime.insert("jpg","image/jpg");
    m_mime.insert("jpeg","image/jpeg");
    m_mime.insert("gif","image/gif");
    m_mime.insert("png","image/png");
    m_mime.insert("avi","video/avi");
    m_mime.insert("mpg","video/mpg");
    m_mime.insert("mpeg","video/mpeg");
    m_mime.insert("mp4","video/mp4");
    m_mime.insert("mkv","video/x-matroska");
    m_mime.insert("qt","video/quicktime");
    m_mime.insert("webm","video/webm");
    m_mime.insert("wmv","video/x-ms-wmv");
    m_mime.insert("flv","video/x-flv");
    m_mime.insert("mp3","audio/mp3");
    m_mime.insert("wav","audio/vnd.wav");
    m_mime.insert("tar","application/x-tar");
    m_mime.insert("zip","application/zip");
    m_mime.insert("pdf","application/pdf");
}

QString HttpConnection::root()
{
    return m_root;
}

void HttpConnection::setRoot(QString path)
{
    qDebug() << this << "Root set to: " << path;
    m_root = path;
}

int HttpConnection::rate()
{
    return m_rate;
}

void HttpConnection::setRate(int value)
{
    qDebug() << this << "Rate set to: " << value;
    m_rate = value;
}

void HttpConnection::processGet(QByteArray data)
{
    qDebug() << this << "Processing GET";

    QByteArray eoh;//End of Header
    /*The end of the HTTP header is are 2 new lines (2 return line feeds)- thats how we know the
     * difference betweenthe header and the payload
     * Typically the payload is not sent in a GET request,
     * but might happen in case of coockies
     */
    eoh.append("\r\n\r\n");

    QString header = m_request.value("svr_header","");
    header.append(data);
    m_request.insert("svr_header", header);

    if(header.contains(eoh))
    {
        //End of header found, parse the header
        QStringList lines = header.split("\r\n");// Split on return line feed
        for(int i = 0; i < lines.count() ; i++)
        {
            QString line = lines.at(i);

            //Processing the first line
            if(i == 0)
            {
                //First line has 3 values METHOD PATH VERSION
                QStringList items = line.split(" ");//Split on space
                if(items.count() >= 1) m_request.insert("http_method", items.at(0).trimmed());

                if(items.count() >= 2)
                {
                    /*"/some&20Adir/somefile.txt"
                     *Relative file path - this is where a lot of attackes happen
                     * an attacker can send /.. or ../.. as a request and there for can reach up the
                     * the path, even to the root directory
                     * think of a fix!
                     */
                    QString path = items.at(1).trimmed();
                    /*The request will come in encoded
                     *so we are taking the endocing out of the request
                     */
                    path = QUrl::fromPercentEncoding(path.toLatin1());
                    m_request.insert("http_path", path);
                }

                if(items.count() >= 3) m_request.insert("http_version", items.at(2).trimmed());
            }
            else
            {
                //All other lines are key value pairs
                if(!line.contains(":") && !line.isEmpty())
                {
                    qWarning() << this << "Skipping line due to missing ':' " << line;
                    continue;
                }

                int pos = line.indexOf(":");
                QString key = line.mid(0,pos);
                QString value = line.mid(pos + 1);
                m_request.insert(key.trimmed(),value.trimmed());
            }

        }

        //attept to handle the request
        handleRequest();
    }

}

void HttpConnection::handleRequest()
{
    qDebug() << this << "Handle the Request";

    //Path from the firt line
    QString file = m_request.value("http_path","");

    //Strip out any directory jumps
    //If an attacker tries to jump a directory we will just return a 404...
    //Defensive coding
    file = file.replace("..","");

    // /mydir/something/something + /test.jpg
    QString actualFile = m_root + file;
    QFileInfo fi(actualFile);
    QByteArray response;

    //If it is a directory, check for index.html
    if(fi.isDir())
    {
        //Building the responce - 200 ok, 404 not found etc...
        qDebug() << this << "client is requesting a directory...";
        QString indexFile = actualFile + "index.html";
        QFileInfo fIndex(indexFile);

        if(fIndex.exists())
        {
            qDebug() << this << "setting / to /index.html";
            fi.setFile(indexFile);
        }
        else
        {
            qWarning() << this << "Index file is missing: " << indexFile;
        }
    }

    //Send the file if it exists
    if(fi.exists() && fi.isFile())
    {
        //YES it exists
        QString mime = getMimeType(fi.fileName());
        qDebug() << this << "Sending file: " << fi.filePath();
        m_response.insert("code", "200");
        m_response.insert("path", fi.filePath());

        response.append("HTTP/1.1 200 OK\r\n");
        response.append("Content-Type: " + mime + "\r\n");
        response.append("Content-Length: " + QString::number(fi.size()) + "\r\n");
        response.append("Connection: close\r\n");
        response.append("\r\n");
    }
    else
    {
        //No it does not exist
        m_response.insert("code", "404");

        response.append("HTTP/1.1 404 NOT FOUND\r\n");
        response.append("Connection: close\r\n");
        response.append("\r\n");
    }
    m_socket->write(response);
}

void HttpConnection::sendFile(QString file)
{
    //Defensive programming
    if(!m_socket) return;

    qDebug() << this << "Sending file " << file;
    //Able to use this as a parent as the instance of the
    //http connection is on the worker thread we created
    m_file = new QFile(file,this);
    m_transfer = new RateTransfer(this);

    qDebug() << this << "Created: " << m_transfer;

    //Connecting signals and slots of the RateTransfer with the http connection
    connect(m_transfer,&RateTransfer::started,this, &HttpConnection::started);
    connect(m_transfer,&RateTransfer::finished,this, &HttpConnection::finished);
    connect(m_transfer,&RateTransfer::error,this, &HttpConnection::transferError);
    connect(m_transfer,&RateTransfer::transfered,this, &HttpConnection::transfered);

    //Defensive programming
    if(!m_file->open(QFile::ReadOnly))
    {
        qWarning() << "Could not open file: " << file;
        m_socket->close();
    }

    //TODO possible put it in to UI
    m_transfer->setSource(m_file);
    m_transfer->setDestination(m_socket);
    m_transfer->setRate(m_rate);
    m_transfer->setSize(1024);

    qDebug() << this << "Starting file transfer...";
    //We need to do this because if we are checking the code while we are sending the file
    //it will try to send a file over and over again going in to a loop
    m_response.remove("code");
    m_transfer->start();
}

QString HttpConnection::getMimeType(QString path)
{
    int pos = path.indexOf(".");
    //application/octet-stream is a default mime type for web traffic
    if(pos <= 0) return "application/octet-stream";

    QString ext = path.mid(pos +1).toLower(); //Getting the file extension
    if(m_mime.contains(ext))
    {
        return m_mime.value(ext);
    }
    return "application/octet-stream";
}

void HttpConnection::connected()
{
    qDebug() << this << "connected" << getSocket();
}

void HttpConnection::disconnected()
{
    qDebug() << this << "disconnected" << getSocket();
}

void HttpConnection::readyRead()
{
    //Defensive programming
    if(!m_socket) return;
    qDebug() << this << "readyRead" << m_socket;
    QByteArray data = m_socket->readAll();
    /*potecial security flaw,
     * the header will keep reading till it reaches the end of the header regardless of the size
     * an attacker can keep feeding the header that cna lead to a crash
     * TODO fix!
     */
    processGet(data);
}

void HttpConnection::bytesWritten(qint64 bytes)
{
    if(!m_socket) return;

    QString code = m_response.value("code", "");

    if(!code.isEmpty())
    {
        qDebug() << this << "code = " << code;
    }

    if(code == "200")
    {
        QString file = m_response.value("path","");
        qDebug() << this << "Attempting to sdend file: " << file;
        sendFile(file);
        //close the socket when done!
    }
    if(code == "404")
    {
        m_socket->close();
        return;
    }
}

void HttpConnection::stateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << this << "stateChanged" << m_socket << socketState;
}

void HttpConnection::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << this << "error" << m_socket << socketError;
}

void HttpConnection::started()
{
    qDebug() << this << "File transfer started!";
}

void HttpConnection::transfered(qint64 bytes)
{
    qDebug() << this << "File transfered " << bytes;
}

void HttpConnection::finished()
{
    qDebug() << this << "File transfer finished";
    //We have to do this to flush the buffer
    m_file->close();
    m_socket->close();
}

void HttpConnection::transferError()
{
   qDebug() << this <<  "File transfer error: " << m_transfer->errorString();
   m_file->close();
   m_socket->close();
}
