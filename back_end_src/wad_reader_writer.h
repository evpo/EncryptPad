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
#include <streambuf>
#include <iterator>
#include <string>
#include "packet_typedef.h"
#include "packet_stream.h"
#include "encryptmsg/emsg_types.h"

namespace EncryptPad
{
    EpadResult ExtractFromWad(RandomInStream &in, OutStream &out, std::string &key_file);
    EpadResult ExtractKeyFromWad(RandomInStream &in, std::string &key_file);
    bool WriteWad(InStream &in, OutStream &out, const std::string &key_file);
    bool WriteWadHead(const std::string &key_file, uint32_t payload_size, const std::string &version, OutStream &out);
    EpadResult ParseWad(RandomInStream &in, std::string &key_file, uint32_t &payload_offset, uint32_t &payload_size);
}
