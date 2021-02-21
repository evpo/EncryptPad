include($$PWD/../fakevim/fakevim.pri)

INCLUDEPATH += $$PWD

SOURCES += $$PWD/editor.cpp
HEADERS += $$PWD/editor.h
CONFIG += qt
QT += widgets
