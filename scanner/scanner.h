#ifndef SCANNER_H
#define SCANNER_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QTextStream>

class Scanner : public QObject
{
    Q_OBJECT
public:
    explicit Scanner(QObject *parent = nullptr);

    void pyCreator(QString path);
private:
    QString filename;
    QFile file;
    QTextStream stream;
};

#endif // SCANNER_H
