include(../example/editor.pri)

SOURCES += fakevim_test.cpp
HEADERS += fakevimplugin.h

CONFIG += qt
CONFIG+=c++2a
QT += widgets
QT += testlib
