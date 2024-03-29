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
#include <QFileOpenEvent>
#include <QMessageBox>
#include <iostream>
#include "application.h"

using namespace std;

Application::Application(int &argc, char **argv):QApplication(argc, argv), mainWindow(nullptr)
{

}

bool Application::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::FileOpen:
        loadFile(static_cast<QFileOpenEvent *>(event)->file());
        return true;

    case QEvent::ApplicationStateChange:
        {
            const auto *stateChangeEvent = static_cast<QApplicationStateChangeEvent *>(event);
            if(stateChangeEvent->applicationState() == Qt::ApplicationActive)
            {
                mainWindow->onApplicationActive();
            }
        }

        return true;
    default:
        return QApplication::event(event);
    }
}

void Application::loadFile(const QString &fileName)
{
    if(mainWindow)
        mainWindow->open(fileName);
}

MainWindow *Application::getMainWindow()
{
    return mainWindow;
}

void Application::setMainWindow(MainWindow *window)
{
    mainWindow = window;
}

ThemeAppearance Application::getAutoThemeAppearance() const
{
    auto palette = this->palette();
    const QColor textColor = palette.color(QPalette::WindowText);
    return (textColor.red() > 128 && textColor.blue() > 128 && textColor.green() > 128) ?
        ThemeAppearance::Dark : ThemeAppearance::Light;
}
