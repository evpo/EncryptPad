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
#ifndef KEY_GENERATION_H
#define KEY_GENERATION_H
#include <string>
#include <stdexcept>

#include "packet_composer.h"

namespace EncryptPad
{
    class IoException : public std::runtime_error
    {
    public:
        IoException(std::string message):std::runtime_error(message){}
        IoException(const char *message):std::runtime_error(message){}
    };

    void GenerateNewKey(unsigned char buffer[], size_t length);
    void GenerateNewKey(const std::string& key_file_path, EncryptPad::EncryptParams *encrypt_params = nullptr, PacketMetadata *metadata = nullptr);
    EncryptPad::PacketMetadata GetDefaultKFMetadata(int iterations);
}
#endif // KEY_GENERATION_H
