//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "passphrase_provider.h"

namespace EncryptMsg
{
    Passphrase::Passphrase(const SafeVector &data_p)
        :data(data_p)
    {
    }
}

