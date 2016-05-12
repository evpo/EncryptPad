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

#include <string>
#include <vector>
#include "packet_stream.h"

namespace EncryptPad
{
    bool OpenFile(const std::string &file_name, InPacketStreamFile &stm);
    bool OpenFile(const std::string &file_name, OutPacketStreamFile &stm);
    bool RemoveFile(const std::string &file_name);

    // The vector is not secure. Only use this method for reading encrypted data
    // or in CLI.
    bool LoadFromIOStream(int file_descriptor, std::vector<byte> &buffer);
    bool SaveToIOStream(int file_descriptor, const Botan::SecureVector<byte> &buffer);
}
