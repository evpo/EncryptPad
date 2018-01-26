//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "passphrase_provider.h"

namespace LibEncryptMsg
{
    Passphrase::Passphrase(const SafeVector &data_p)
        :data(data_p)
    {
    }
}

