#pragma once
#include <memory>
#include <string>
#include "key_service.h"
#include "encryptmsg/passphrase_provider.h"
#include "encryptmsg/algo_spec.h"

namespace EncryptPad
{
    class KeyServiceKeyProvider : public LibEncryptMsg::SymmetricKeyProvider
    {
        private:
            KeyService *key_service_;
            const std::string *passphrase_;
        public:
            KeyServiceKeyProvider(KeyService *key_service, const std::string *passphrase);

            std::unique_ptr<LibEncryptMsg::EncryptionKey> GetKey(
                    LibEncryptMsg::CipherAlgo cipher_algo,
                    LibEncryptMsg::HashAlgo hash_algo,
                    uint8_t iterations,
                    LibEncryptMsg::Salt salt,
                    std::string description,
                    bool &cancelled) override;
    };
}

