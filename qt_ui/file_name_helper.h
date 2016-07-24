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
#pragma once
#include <QString>

QString GetOpenDialogFilter();
QString GetSaveDialogFilter();
QString GetKeyDialogFilter();
QString GetAllFilesFilter();

QString AppendExtensionForFileDialog(QString fileName, QString selectedFilter);

bool IsEncryptPadFormat(const QString &fileName);
bool IsGpgFormat(const QString &fileName);
bool IsCryptPadFormat(const QString &fileName);
QString GetFileFilterFromFileName(const QString &fileName);
QString GetLibcurlFilter();

struct FlagResetter
{
    private:
        bool *flag_;
        bool unset_state_;
    public:
        FlagResetter(bool &flag, bool unset_state = false):flag_(&flag), unset_state_(unset_state)
    {
        *flag_ = true;
    }
        ~FlagResetter()
        {
            if(flag_)
                *flag_ = unset_state_;
        }

        void Disconnect()
        {
            flag_ = nullptr;
        }
};
