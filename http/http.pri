QT += network
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/httpconnection.h \
    $$PWD/httpserver.h \
    $$PWD/ratetransfer.h

SOURCES += \
    $$PWD/httpconnection.cpp \
    $$PWD/httpserver.cpp \
    $$PWD/ratetransfer.cpp
