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
#include "algo_spec.h"

namespace EncryptPad
{
    const AlgoSpec &GetAlgoSpec(CipherAlgo cipher_algo)
    {
        switch(cipher_algo)
        {
            case CipherAlgo::TripleDES:
                static AlgoSpec triple_des =
                {
                    CipherAlgo::TripleDES,
                    "3DES/CFB",
                    8,
                    192
                };
                return triple_des;

            case CipherAlgo::CAST5:

                static AlgoSpec cast5 =
                {
                    CipherAlgo::CAST5,
                    "CAST5/CFB",
                    8,
                    128
                };
                return cast5;

            case CipherAlgo::AES128:

                static AlgoSpec aes128 =
                {
                    CipherAlgo::AES128,
                    "AES-128/CFB",
                    16,
                    128
                };
                return aes128;

            case CipherAlgo::AES256:

                static AlgoSpec aes256 =
                {
                    CipherAlgo::AES256,
                    "AES-256/CFB",
                    16,
                    256
                };
                return aes256;
            default:
                static AlgoSpec unknown =
                {
                    CipherAlgo::Unknown,
                    "",
                    -1,
                    -1
                };
                return unknown;
        }
    }

    const HashSpec &GetHashSpec(HashAlgo hash_algo)
    {
        switch(hash_algo)
        {
            case HashAlgo::SHA160:
                static HashSpec sha160 =
                {
                    HashAlgo::SHA160,
                    "OpenPGP-S2K(SHA-160)"
                };
                return sha160;
            case HashAlgo::SHA256:
                static HashSpec sha256 =
                {
                    HashAlgo::SHA256,
                    "OpenPGP-S2K(SHA-256)"
                };
                return sha256;
            default:
                static HashSpec unknown =
                {
                    HashAlgo::Unknown,
                    ""
                };
                return unknown;
        }
    }
}

