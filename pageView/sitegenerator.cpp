#include "sitegenerator.h"

SiteGenerator::SiteGenerator(QObject *parent) : QObject(parent)
{

}

void SiteGenerator::indexGenerator(QString path)
{
    QString filename = path + "/index.html";
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    QTextStream stream(&file);

}
