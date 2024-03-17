//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//This file is part of EncryptPad
//
//EncryptPad is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 2 of the License, or
//(at your option) any later version.
//
//EncryptPad is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EncryptPad.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************
#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include "mainwindow.h"

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    void setMainWindow(MainWindow *);
    MainWindow *getMainWindow();
    ThemeAppearance getThemeAppearance() const;
protected:
    bool event(QEvent *);
    void loadFile(const QString &fileName);
private:
    MainWindow *mainWindow;
signals:
    
public slots:
    
};

#endif // APPLICATION_H
