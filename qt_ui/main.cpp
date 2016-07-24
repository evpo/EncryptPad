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
#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include "mainwindow.h"
#include "application.h"



int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(EncryptPad);

    Application app(argc, argv);
    //QTextCodec::setCodecForTr(QTextCodec::codecForName("Unicode"));

    QTranslator translator;
    translator.load("encryptpad_ru_ru");

    app.installTranslator(&translator);

    app.setOrganizationName("Evpo"); //
    app.setApplicationName("EncryptPad");

	QString fileName;
	if(argc > 1)
		fileName = argv[1];
    MainWindow mainWin;
    app.setMainWindow(&mainWin);
#if defined(Q_OS_SYMBIAN)
    mainWin.showMaximized();
#else
    mainWin.show();
#endif

    if(!fileName.isEmpty())
        mainWin.open(fileName);

    return app.exec();
}
//! [0]
