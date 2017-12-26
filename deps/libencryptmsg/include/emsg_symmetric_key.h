#pragma once
#include <memory>
#include "emsg_types.h"
#include "passphrase_provider.h"

namespace LibEncryptMsg
{
    Salt GenerateRandomSalt();

    std::unique_ptr<EncryptionKey> GenerateEncryptionKey(const Passphrase &passphrase, CipherAlgo cipher_algo, HashAlgo hash_algo, uint8_t iterations, Salt salt);

    std::unique_ptr<EncryptionKey> GenerateEncryptionKey(const Passphrase &passphrase, unsigned key_size, HashAlgo hash_algo, uint8_t iterations, Salt salt);
}

