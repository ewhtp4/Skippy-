#ifndef RATETRANSFER_H
#define RATETRANSFER_H

#include <QObject>
#include <QIODevice>
#include <QTimer>
#include <QTime>
#include <QDebug>
/*Able to choose a source IO device and a destination IO device
 *For prefomance, once we reach a certain number of bytes per second
 *we stop the transfer and schedule the trasfer,
 *the transfer will continue once the timmer(singleShot) expire
 */
class RateTransfer : public QObject
{
    Q_OBJECT
public:
    explicit RateTransfer(QObject *parent = nullptr);
    //What rate
    int rate();
    void setRate(int value);
    //What size/buffer
    int size();
    void setSize(int value);
    //Whats the maximum number of bytes(gives us a choice to send only part of a file)
    qint64 maximum();
    void setMaximum(qint64 value);

    QIODevice *source();
    void setSource(QIODevice *device);

    QIODevice *destination();
    void setDestination(QIODevice *device);

    bool isTransfering();
    QString errorString();

protected: //Only for this class and classes that inherit this class
    int m_rate;
    int m_size;
    bool m_transfering;
    qint64 m_maximum;
    qint64 m_transfered;//Number of bytes we currently transfered
    QIODevice *m_source;
    QIODevice *m_destination;
    QTimer m_timer;
    QString m_error;
    bool m_scheduled;

    void setDefaults();
    bool checkDevices();
    bool checkTransfer();
    void scheduleTransfer();

signals:
    void started();
    void transfered(qint64 bytes); //qint64 - integer big enough to hold a size of a file
    void finished();
    void error();

public slots:
    void start();
    void stop();

protected slots:
    void transfer();
    void readyRead();
    void bytesWritten(qint64 bytes);
};

#endif // RATETRANSFER_H
