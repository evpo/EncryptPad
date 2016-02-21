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
#include "file_helper.h"
#include <cstdio>
#include "win_file_reader.h"
#include "epad_utilities.h"

namespace EncryptPad
{
    bool OpenFile(const std::string &file_name, InPacketStreamFile &stm)
    {
#if defined(__MINGW__) || defined(__MINGW32__)
        FileHndl file = OpenInputWin(file_name);
#else
        FileHndl file(fopen(file_name.data(), "rb"));
#endif

        if(!file)
            return false;

        if(fseek(file.get(), 0, SEEK_END))
            return false;

        stream_length_type length = ftell(file.get());
        if(fseek(file.get(), 0, SEEK_SET))
            return false;

        stm.Set(file, length);
        return true;
    }

    bool OpenFile(const std::string &file_name, OutPacketStreamFile &stm)
    {
#if defined(__MINGW__) || defined(__MINGW32__)
        FileHndl file = OpenOutputWin(file_name);
#else
        FileHndl file(fopen(file_name.data(), "wb"));
#endif
        if(!file)
            return false;

        stm.Set(file);
        return true;
    }

    bool RemoveFile(const std::string &file_name)
    {
#if defined(__MINGW__) || defined(__MINGW32__)
        return RemoveFileWin(file_name);
#else
        return remove(file_name.data()) == 0;
#endif
    }
}
