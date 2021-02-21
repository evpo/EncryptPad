include($$PWD/utils/utils.pri)

INCLUDEPATH += $$PWD

SOURCES += $$PWD/fakevimhandler.cpp \
           $$PWD/fakevimactions.cpp

HEADERS += $$PWD/fakevimhandler.h \
           $$PWD/fakevimactions.h

CONFIG += qt
QT += widgets
