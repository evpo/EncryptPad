//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "openpgp_conversions.h"
#include "botan/pgp_s2k.h"

using namespace Botan;

namespace LibEncryptMsg
{
    size_t DecodeS2KIterations(uint8_t count)
    {
        return OpenPGP_S2K::decode_count(count);
    }

    uint8_t EncodeS2KIterations (size_t iterations)
    {
        return OpenPGP_S2K::encode_count(iterations);
    }
}
