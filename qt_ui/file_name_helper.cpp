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
        {".cpad", "EncryptPad Alpha (*.cpad)"},
        {".txt", "Plain Text (*.txt)"},
        {nullptr, nullptr}
    };

    const char *sOpenDialogFilter = "Encrypted Files (*.epd *.gpg *.cpad);; Plain Text (*.txt)";
    const char *sKeyDialogFilter = "Key (*.key)";

#if defined(__APPLE__) || defined(unix) || defined(__unix__) || defined(__unix)
    const char *sAllFilesFilter = "All Files (*)";
#else // WINDOWS
    const char *sAllFilesFilter = "All Files (*.*)";
#endif

#if defined(__APPLE__) || defined(unix) || defined(__unix__) || defined(__unix)
    const char *kLibcurlFilter = "*";
#else
    const char *kLibcurlFilter = "*.exe";
#endif

}

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__APPLE__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
    QString AppendExtensionForFileDialog(QString fileName, QString selectedFilter)
    {
        return fileName;
    }
#pragma GCC diagnostic pop
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
            {"(*.cpad)", ".cpad"},
            {"(*.gpg)", ".gpg"},
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

const char *GetFiltersWithoutAllFiles()
{
    static std::string str;
    if(str.empty())
    {
        const FileFormat *format = fileFormats;
        while(format->ext)
        {
            if(!str.empty())
                str.append(";; ");

            str.append(format->filter);
            format++;
        }
    }

    return str.c_str();
}

const char *AppendAllFilesIfNeeded(std::string &buf, const char *otherFiles)
{
    if(buf.empty())
    {
        buf += otherFiles;
        buf += ";; ";
        buf += sAllFilesFilter;
    }
    return buf.c_str();
}

const char *GetSaveDialogFilter()
{
    static std::string buf;
    return AppendAllFilesIfNeeded(buf, GetFiltersWithoutAllFiles());
}

const char *GetOpenDialogFilter()
{
    static std::string buf;
    return AppendAllFilesIfNeeded(buf, sOpenDialogFilter);
}

const char *GetKeyDialogFilter()
{
    static std::string buf;
    return AppendAllFilesIfNeeded(buf, sKeyDialogFilter);
}

const char *GetAllFilesFilter()
{
    return sAllFilesFilter;
}

bool IsEncryptPadFormat(const QString &fileName)
{
    return fileName.endsWith(".epd", Qt::CaseInsensitive);
}

bool IsGpgFormat(const QString &fileName)
{
    return fileName.endsWith(".gpg", Qt::CaseInsensitive);
}

bool IsCryptPadFormat(const QString &fileName)
{
    return fileName.endsWith(".cpad", Qt::CaseInsensitive);
}

QString GetFileFilterFromFileName(const QString &fileName)
{
    const FileFormat *table = fileFormats;
    while(table->ext)
    {
        if(fileName.endsWith(QString(table->ext), Qt::CaseInsensitive))
            return QString(table->filter);

        table ++;
    }

    return QString(GetAllFilesFilter());
}

QString GetLibcurlFilter()
{
    return QString(kLibcurlFilter);
}
