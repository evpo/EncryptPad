#pragma once
#include <memory>
#include <string>
#include "emsg_types.h"
#include "algo_spec.h"
#include "emsg_mem.h"

namespace LibEncryptMsg
{
    class PassphraseProvider
    {
        public:
            virtual ~PassphraseProvider() {}
            virtual std::unique_ptr<SecureVector> GetPassphrase(std::string description, bool &canceled) = 0;
    };

    class SymmetricKeyProvider
    {
        public:
            virtual ~SymmetricKeyProvider() {}
            virtual std::unique_ptr<EncryptionKey> GetKey(CipherAlgo cipher_algo, HashAlgo hash_algo, uint8_t iterations, Salt salt,
                    std::string description, bool &canceled) = 0;
    };

    struct Passphrase
    {
        const SecureVector data;
        Passphrase(const SecureVector &data_p);
    };

}

