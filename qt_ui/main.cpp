//**********************************************************************************
//EncryptPad Copyright 2021 Evgeny Pokhilko 
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
//#include <QTextCodec>
#include <QTranslator>
#include <QResource>
#include <QDirIterator>
#include <QLocale>
#include <QQueue>
#include <QFile>
#include <QFileInfo>
#include <string>
#include <exception>
#include "mainwindow.h"
#include "application.h"
#include "repository.h"
#include "diagnostic_log.h"
#include "plog/Log.h"

namespace
{
    QString findLangResource(const QString &prefix, const QString &userLang)
    {
        QString empty;
        QString lowerUserLang = userLang.toLower();
        if(lowerUserLang.indexOf('-') != -1)
        {
            lowerUserLang = lowerUserLang.left(lowerUserLang.indexOf('-'));
        }

        if(lowerUserLang.length() != 2)
            return empty;

        QDirIterator it(":/cultures/");
        while(it.hasNext())
        {
            QString fullPath = it.next();
            QString lang = it.fileName();

            if(!lang.startsWith(prefix))
                continue;

            lang = lang.right(lang.length() - prefix.length());
            if(lang.indexOf('_') == -1)
                continue;

            lang = lang.left(lang.indexOf('_'));
            if(lang.length() != 2)
                continue;

            if(lang == lowerUserLang)
                return fullPath;
        }

        return empty;
    }

    struct CommandArguments
    {
        QString fileName;
        QString language;
        QString logFile;
        QString logSeverity;
        bool isHelpRequested = false;
    };

    CommandArguments parseArguments(const Application &app)
    {
        CommandArguments empty;
        CommandArguments retVal;
        int argCount = app.arguments().length();
        if(argCount == 0)
            return retVal;

        QQueue<QString> queue;
        queue.append(app.arguments());
        // remove the executable
        queue.pop_front();
        while(!queue.isEmpty())
        {
            QString arg = queue.front();
            if(arg == "--lang")
            {
                queue.pop_front();
                if(queue.isEmpty())
                    return empty;

                retVal.language = queue.front();
            }
            else if(arg == "--log-file")
            {
                queue.pop_front();
                if(queue.isEmpty())
                    return empty;

                retVal.logFile = queue.front();

            }
            else if(arg == "--log-severity")
            {
                queue.pop_front();
                if(queue.isEmpty())
                    return empty;

                retVal.logSeverity = queue.front();
            }
            else if(arg == "--help")
            {
                retVal.isHelpRequested = true;
            }
            else
            {
                retVal.fileName = queue.front();
            }
            queue.pop_front();
        }

        return retVal;
    }
}

bool loadTranslatorResource(Application &app, QTranslator &translator, QResource &resource)
{
    bool result = translator.load(resource.data(), resource.size());
    assert(result);
    result |= app.installTranslator(&translator);
    return result;
}

bool loadTranslatorResource(Application &app, QTranslator &translator, const QString &path)
{
    QResource resource(path);
    return loadTranslatorResource(app, translator, resource);
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(EncryptPad);

    Application app(argc, argv);
    CommandArguments arguments = parseArguments(app);
    assert(argc >= 1);

    if(!arguments.logFile.isEmpty())
    {
        plog::init(EncryptPad::ParsePlogSeverity(arguments.logSeverity.toStdString()),
                arguments.logFile.toStdString().c_str());
        LOG_INFO << "Log instance started";
    }

    try
    {
        EncryptPad::InitializeRepositoryPath(argv[0]);

        QStringList userLangs;
        if(arguments.language.isEmpty())
        {
            userLangs.append(QLocale::system().uiLanguages());
        }
        else
        {
            LOG_INFO << "language: " << arguments.language.toStdString();
            userLangs.append(arguments.language);
        }
        QString resourcePath;
        QString fakeVimResourcePath;
        QString qtExcerptPath;
        for(QString userLang : userLangs)
        {
            if(userLang == "en" || userLang.startsWith("en-"))
                break;
            resourcePath = findLangResource("encryptpad_", userLang);
            LOG_INFO << "resourcePath is [" << resourcePath.toStdString() << "]";
            if(resourcePath.isEmpty())
            {
                continue;
            }

            fakeVimResourcePath = findLangResource("fakevim_", userLang);
            LOG_INFO << "fakeVimResourcePath is [" << fakeVimResourcePath.toStdString() << "]";
            assert(!fakeVimResourcePath.isEmpty());

            qtExcerptPath = findLangResource("qt_excerpt_", userLang);
            LOG_INFO << "qtExcerptPath is [" << qtExcerptPath.toStdString() << "]";
            assert(!qtExcerptPath.isEmpty());
            break;
        }

        QTranslator translator;
        QTranslator fakeVimTranslator;
        QTranslator excerptTranslator;

        if(!resourcePath.isEmpty())
        {
            bool result = loadTranslatorResource(app, translator, resourcePath);
            assert(result);
            (void)result;
        }

        if(!fakeVimResourcePath.isEmpty())
        {
            bool result = loadTranslatorResource(app, fakeVimTranslator, fakeVimResourcePath);
            assert(result);
            (void)result;
        }

        if(!qtExcerptPath.isEmpty())
        {
            bool result = loadTranslatorResource(app, excerptTranslator, qtExcerptPath);
            assert(result);
            (void)result;
        }

        app.setOrganizationName("Evpo"); //
        app.setApplicationName("EncryptPad");

        MainWindow mainWin;
        mainWin.setAutoThemeAppearance(app.getAutoThemeAppearance());
        app.setMainWindow(&mainWin);
        mainWin.show();

        if(!arguments.fileName.isEmpty())
        {
            if(!QFile::exists(arguments.fileName))
            {
                QFile f(arguments.fileName);
                if(!f.open(QIODevice::WriteOnly))
                {
                    LOG_WARNING << "The file does not exist. Cannot create the file : " << arguments.fileName;
                }
            }
            QFileInfo info(arguments.fileName);
            mainWin.open(info.absoluteFilePath());
        }

        if(arguments.isHelpRequested)
            mainWin.showHelp();

        return app.exec();
    }
    catch(const std::exception &ex)
    {
        LOG_ERROR << "Critical error. Exception message: " <<  ex.what();
        LOG_ERROR << "Exit code: 1";
        return 1;
    }
}
