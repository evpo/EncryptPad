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
#include "recent_files_service.h"

RecentFilesService::RecentFilesService(QWidget *parent)
    :signal_mapper_(parent), parent_(parent), menu_(nullptr), begin_index_(0), end_(nullptr), max_files_(0), file_count_(0)
{
        connect(&signal_mapper_, SIGNAL(mapped(const QString &)), this, SLOT(SignalMapped(const QString &)));
}

void RecentFilesService::SignalMapped(const QString &str)
{
    emit FileSelected(str);
}
