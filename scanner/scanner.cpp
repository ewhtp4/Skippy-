#include "scanner.h"

Scanner::Scanner(QObject *parent) : QObject(parent)
{

}

void Scanner::pyCreator(QString path)
{
    QString filename = path + "/scanner.py";
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    QTextStream stream(&file);

    stream << '# Importing the libraries\n';
    stream << 'import sys\n';
    stream << 'import socket\n';
    stream << 'import subprocess\n';
    stream << 'import os\n';
    stream << 'import time\n';
    stream << 'import signal\n';
    stream << 'import random\n';
    stream << 'import string\n';
    stream << 'import threading\n';
    stream << 'import re\n';
    stream << 'from urlparse import urlsplit\n';
}
