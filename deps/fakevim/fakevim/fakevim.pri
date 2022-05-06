include($$PWD/utils/utils.pri)

INCLUDEPATH += $$PWD

SOURCES += $$PWD/fakevimhandler.cpp \
           $$PWD/fakevimactions.cpp \
           $$PWD/fakevimsavedaction.cpp

HEADERS += $$PWD/fakevimhandler.h \
           $$PWD/fakevimactions.h \
           $$PWD/fakevimsavedaction.h

CONFIG += qt
QT += widgets
