#ifndef SITEGENERATOR_H
#define SITEGENERATOR_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QTextStream>

class SiteGenerator : public QObject
{
    Q_OBJECT
public:
    explicit SiteGenerator(QObject *parent = nullptr);

    void indexGenerator(QString Path);
private:
    QString filename;
    QFile file;
    QTextStream stream;
};

#endif // SITEGENERATOR_H
