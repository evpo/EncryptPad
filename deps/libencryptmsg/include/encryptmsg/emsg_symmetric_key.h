//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <memory>
#include "emsg_types.h"
#include "passphrase_provider.h"

namespace EncryptMsg
{
    Salt GenerateRandomSalt();

    //TODO: iterations are encoded. It's dangerous to leave them with this type because the parameter can take int without a warning.
    std::unique_ptr<EncryptionKey> GenerateEncryptionKey(const Passphrase &passphrase, CipherAlgo cipher_algo, HashAlgo hash_algo, uint8_t iterations, Salt salt);

    std::unique_ptr<EncryptionKey> GenerateEncryptionKey(const Passphrase &passphrase, unsigned key_size, HashAlgo hash_algo, uint8_t iterations, Salt salt);
}

