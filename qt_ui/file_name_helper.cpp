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
#include "file_name_helper.h"
#include <QtGlobal>
#include <QApplication>

namespace
{
    struct FileFormat
    {
        const char *ext;
        const char *filter;
    };

    const FileFormat fileFormats[] =
    {
        {".epd", "EncryptPad (*.epd)"},
        {".gpg", "GnuPG (*.gpg)"},
        {".asc", QT_TRANSLATE_NOOP("FileExtensions", "ASCII-armored GnuPG (*.asc)")},
        {".txt", QT_TRANSLATE_NOOP("FileExtensions", "Plain Text (*.txt)")},
        {nullptr, nullptr}
    };

    const char *sOpenDialogFilter = QT_TRANSLATE_NOOP("FileExtensions", "Encrypted Files (*.epd *.gpg *.asc);; Plain Text (*.txt)");
    const char *sKeyDialogFilter = QT_TRANSLATE_NOOP("FileExtensions", "Key (*.key)");

#if defined(__APPLE__) || defined(unix) || defined(__unix__) || defined(__unix)
    const char *sAllFilesFilter = QT_TRANSLATE_NOOP("FileExtensions", "All Files (*)");
#else // WINDOWS
    const char *sAllFilesFilter = QT_TRANSLATE_NOOP("FileExtensions", "All Files (*.*)");
#endif

#if defined(__APPLE__) || defined(unix) || defined(__unix__) || defined(__unix)
    const char *kLibcurlFilter = "*";
#else
    const char *kLibcurlFilter = "*.exe";
#endif

    QString AppendAllFiles(const QString &otherFiles)
    {
        QString buf;
        buf += otherFiles;
        buf += ";; ";
        buf += qApp->translate("FileExtensions", sAllFilesFilter);
        return buf;
    }

    const QString &GetFiltersWithoutAllFiles()
    {
        static QString str;
        if(str.isEmpty())
        {
            const FileFormat *format = fileFormats;
            while(format->ext)
            {
                if(!str.isEmpty())
                    str.append(";; ");

                str.append(qApp->translate("FileExtensions", format->filter));
                format++;
            }
        }

        return str;
    }
}

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__APPLE__)
    QString AppendExtensionForFileDialog(QString fileName, QString selectedFilter)
    {
        (void)selectedFilter;
        return fileName;
    }
#endif

#if defined(unix) || defined(__unix__) || defined(__unix)
    QString AppendExtensionForFileDialog(QString fileName, QString selectedFilter)
    {
        if(selectedFilter.isEmpty())
            return fileName;

        struct Filter2Ext
        {
            const char *filter;
            const char *ext;
        };

        static Filter2Ext tbl[] =
        {
            {"(*.epd)", ".epd"},
            {"(*.gpg)", ".gpg"},
            {"(*.asc)", ".asc"},
            {"(*.txt)", ".txt"},
            {"(*.key)", ".key"},
            {nullptr, nullptr},
        };

        Filter2Ext *p = tbl;

        for(;p->ext != nullptr; p++)
        {
            if(selectedFilter.endsWith(QString(p->filter)) && !fileName.toLower().endsWith(p->ext))
                fileName += p->ext;
        }

        return fileName;
    }
#endif


QString GetSaveDialogFilter()
{
    return AppendAllFiles(GetFiltersWithoutAllFiles());
}

QString GetOpenDialogFilter()
{
    return AppendAllFiles(qApp->translate("FileExtensions", sOpenDialogFilter));
}

QString GetKeyDialogFilter()
{
    return AppendAllFiles(qApp->translate("FileExtensions", sKeyDialogFilter));
}

QString GetAllFilesFilter()
{
    return qApp->translate("FileExtensions", sAllFilesFilter);
}

bool IsEncryptPadFormat(const QString &fileName)
{
    return fileName.endsWith(".epd", Qt::CaseInsensitive);
}

bool IsArmorFormat(const QString &fileName)
{
    return fileName.endsWith(".asc", Qt::CaseInsensitive);
}

bool IsGpgFormat(const QString &fileName)
{
    return fileName.endsWith(".gpg", Qt::CaseInsensitive) || fileName.endsWith(".asc", Qt::CaseInsensitive);;
}

QString GetFileFilterFromFileName(const QString &fileName)
{
    const FileFormat *table = fileFormats;
    while(table->ext)
    {
        if(fileName.endsWith(QString(table->ext), Qt::CaseInsensitive))
            return qApp->translate("FileExtensions", table->filter);

        table ++;
    }

    return GetAllFilesFilter();
}

QString GetLibcurlFilter()
{
    return QString(kLibcurlFilter);
}
