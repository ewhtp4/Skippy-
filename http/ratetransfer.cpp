#include "ratetransfer.h"

RateTransfer::RateTransfer(QObject *parent) : QObject(parent)
{
    qDebug() << this << "Created";
    setDefaults();
}

int RateTransfer::rate()
{
    return m_rate;
}

void RateTransfer::setRate(int value)
{
    m_rate = value;
    qDebug() << this << "Rate set to" << value;
}

int RateTransfer::size()
{
    return m_size;
}

void RateTransfer::setSize(int value)
{
    m_size = value;
    qDebug() << this << "Size set to" << value;
}

qint64 RateTransfer::maximum()
{
    return m_maximum;
}

void RateTransfer::setMaximum(qint64 value)
{
    m_maximum = value;
    qDebug() << this << "Maximum set to" << value;
}

QIODevice *RateTransfer::source()
{
    return m_source;
}

void RateTransfer::setSource(QIODevice *device)
{
    m_source = device;
    qDebug() << this << "Source set to" << device;
    //isSequential() - Goes one after the other with no ability to go back to the previous -- you can only go forward
    //if it's Sequential it's a socket if not it's a file - we want a socket...(it could be ehter or)
    //If the source is Sequential connect signals and slots
    if(m_source->isSequential()) connect(m_source,&QIODevice::readyRead, this, &RateTransfer::readyRead);
}

QIODevice *RateTransfer::destination()
{
    return m_destination;
}

void RateTransfer::setDestination(QIODevice *device)
{
    m_destination = device;
    qDebug() << this << "Destination set to" << device;
    //Same as in setSource()
    //except we want to wait for the bytes to be written in to the socket and then ask for more if available
    //**Windowing**
    if(m_destination->isSequential()) connect(m_source,&QIODevice::bytesWritten, this, &RateTransfer::bytesWritten);
}

bool RateTransfer::isTransfering()
{
    return m_transfering;
}

QString RateTransfer::errorString()
{
    return m_error;
}

void RateTransfer::setDefaults()
{
    //TODO Make an interface to give an ability to change this
    qDebug() << this << "Setting the defaults";
    m_rate = 0;
    m_size = 1024;
    m_maximum = 0;
    m_transfering = false;
    m_transfered = 0;
    m_source = 0;
    m_destination = 0;
    m_error = "";
    m_scheduled = false;
    m_timer.setInterval(5);
}

bool RateTransfer::checkDevices()
{
    //Check for source
    //returns false if source or desitinacion is missing
    //or if the connection to source or destination is closed
    if(!m_source) //No source devic4e
    {
        m_transfering = false;
        m_error = "No source device!";
        qDebug() << this << m_error;
        emit error();
        return false;
    }

    if(!m_destination) //no destination device
    {
        m_transfering = false;
        m_error = "No destination device!";
        qDebug() << this << m_error;
        emit error();
        return false;
    }

    if(!m_source->isOpen() || !m_source->isReadable()) //The source is not open or it's not readable
    {
        m_transfering = false;
        m_error = "Source device is not open or is not readable!";
        qDebug() << this << m_error;
        emit error();
        return false;
    }

    if(!m_destination->isOpen() || !m_destination->isWritable()) //The destination is not open or it's not writable
    {
        m_transfering = false;
        m_error = "Destination device is not open or is not writable!";
        qDebug() << this << m_error;
        emit error();
        return false;
    }

    return true;
}

bool RateTransfer::checkTransfer()
{
    if(!m_transfering)
    {
        m_error = "Not transfering, aborting!";
        qDebug() << this << m_error;
        emit error();
        return false;
    }

    if(m_transfered >= m_rate)
    {
        m_error = "Rate exeeded, not allowed to transfer!";
        qDebug() << this << m_error;
        emit error();
        return false;
    }

    return true;
}

void RateTransfer::scheduleTransfer()
{
    //We are using this function for two reasons
    //We dont want to interupt a currant transfare
    //and we dont want to transfer to fast and clog up the connection
    qDebug() << this << "scheduleTransfer called";

    if(m_scheduled) //if all ready schedualed
    {
        qWarning() << this << "Exiting scheduleTransfer due to: waiting on timer";
        return;
    }

    if(!m_transfering) //if not transfering
    {
        qWarning() << this << "Exiting scheduleTransfer due to: not transfering";
        return;
    }

    if(m_source->bytesAvailable() <= 0) //If no bytes available
    {
        qWarning() << this << "Exiting scheduleTransfer due to: no bytes available to be read";
        return;
    }

    int prediction = m_transfered + m_size; //Predicting the final size after the buffer
    if(prediction <= m_rate) //If prediction is less the our limit
    {
        qDebug() << this << "calling transfer from scheduleTransfer";
        transfer();
    }
    else //Otherwise we delay
    {
        int current = QTime::currentTime().msec();
        int delay = 1000 - current;
        qDebug() << this << "Rate limit (" << m_rate << ") exeeded in prediction (" << m_transfered << " to " <<  prediction << "), delaying transfer for " << delay << "ms";
        m_transfered = 0;
        m_scheduled = true;
        m_timer.singleShot(delay,this,&RateTransfer::transfer); //set the timer for delay
    }

}

void RateTransfer::start()
{
    qDebug() << this << "Start called";
    if(m_transfering)
    {
        qDebug() << "Already Transfering!";
        return;
    }

    m_error = "";
    if(!checkDevices()) return;

    m_transfering = true;
    m_transfered = 0;
    emit started();

    //QFile is a random access IODeevice and it will not emmit a readyRead(we need to do it manualy)
    //There for we need to check if its a Qfile(!m_source->isSequential())
    //and if there is data to read(m_source->bytesAvailable() > 0)
    //if so schedule a transfare
    if(!m_source->isSequential() && m_source->bytesAvailable() > 0)
    {
        qDebug() << this << "Starting transfer by calling scheduleTransfer";
        scheduleTransfer();
    }
}

void RateTransfer::stop()
{
    qDebug() << this << "Stopping the transfer";
    m_timer.stop();
    m_transfering = false;
}

void RateTransfer::transfer()
{
    m_scheduled = false;
    qDebug() << this << "transfering at a maximum of " << m_rate << "bytes per second";
    m_error = "";

    //Defensive programming
    if(!checkDevices()) return;
    if(!checkTransfer()) return;

    qDebug() << this << "reading from source";
    QByteArray buffer;
    buffer = m_source->read(m_size);

    qDebug() << this << "writting to destination: " << buffer.length();
    m_destination->write(buffer);
    m_transfered += buffer.length();
    emit transfered(m_transfered);

    if(m_maximum > 0  && m_transfered >= m_maximum)
    {
        qDebug() << this << "Stopping due to maximum limit reached";
        emit finished();
        stop();
    }

    if(!m_source->isSequential() && m_source->bytesAvailable() == 0) //if its a socket and no bytes available
    {
        qDebug() << this << "Stopping due to end of file";
        emit finished();
        stop();
    }

    if(m_transfering == false) return;
    if(!m_source->isSequential() && m_source->bytesAvailable() > 0) //and more bytes available
    {
        qDebug() << this << "Source still has bytes, scheduling a transfer";
        scheduleTransfer();
    }
}

void RateTransfer::readyRead()
{
    qDebug() << this << "readyRead() signaled";
    scheduleTransfer();
}

void RateTransfer::bytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes)
    qDebug() << this << "bytesWritten() signaled";
    scheduleTransfer();
}
