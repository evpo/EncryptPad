//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include "botan/secmem.h"

namespace EncryptMsg
{
    using SafeVector = Botan::secure_vector<uint8_t>;
}
