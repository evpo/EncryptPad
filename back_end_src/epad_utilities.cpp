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
#include "epad_utilities.h"
#include <fstream>
#include "file_system.hpp"
#include "win_file_reader.h"
#include "packet_typedef.h"
#include "packet_stream.h"
#include "file_helper.h"

namespace EncryptPad
{
    bool LoadStringFromFile(const std::string &file_name, std::string &content)
    {
        InPacketStreamFile stm;
        if(OpenFile(file_name, stm) != OpenFileResult::OK)
            return false;

        content.resize(stm.GetCount());
        stm.Read(reinterpret_cast<byte*>(&content[0]), content.size());
        return true;
    }
}
