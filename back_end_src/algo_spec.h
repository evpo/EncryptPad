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

namespace EncryptPad
{
    enum class Compression
    {
        Uncompressed = 0,
        ZIP = 1,
        ZLIB = 2,
        BZip2 = 3,
    };
        
    enum class CipherAlgo
    {
        Unknown = -1,
        TripleDES = 2,
        CAST5 = 3,
        AES128 = 7,
        AES256 = 9,
    };

    enum class HashAlgo
    {
        Unknown = -1,
        SHA160 = 2,
        SHA256 = 8,
        SHA512 = 10,
    };

    struct AlgoSpec
    {
        CipherAlgo cipher_algo;
        std::string botan_name;
        int block_size;
        int key_size;
    };
    
    struct HashSpec
    {
        HashAlgo hash_algo;
        std::string botan_name;
    };

    const AlgoSpec &GetAlgoSpec(CipherAlgo cipher_algo);
    const HashSpec &GetHashSpec(HashAlgo hash_algo);

    const HashAlgo kDefaultHashAlgo = HashAlgo::SHA256;
    const CipherAlgo kDefaultCipherAlgo = CipherAlgo::AES256;
    const int kDefaultIterations = 1015808;
    const int kMaxIterations = 65011712;
    const int kDefaultKeyFileKeyLength = 512 >> 3;
    const Compression kDefaultCompression = Compression::ZLIB; 
}
