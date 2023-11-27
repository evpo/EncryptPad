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
#if defined(__MINGW__) || defined(__MINGW32__)

#include <string>
#include <iostream>
#include <memory>
#include "packet_typedef.h"
#include "epad_utilities.h"

namespace EncryptPad
{
    FileHndl OpenOutputWin(const std::string &file_name);
    FileHndl OpenInputWin(const std::string &file_name);
    bool RemoveFileWin(const std::string &file_name);
    std::string GetLastErrorWin();
    std::string ExpandVariablesWin(const std::string &path);
    stream_length_type WinFTell(FileHndl &file);
    int WinFSeek(FileHndl &file, stream_length_type offset, int origin);
}
#endif


