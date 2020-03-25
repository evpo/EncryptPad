//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once

#include <string>
#include "emsg_types.h"
#include "emsg_utility.h"
#include "emsg_exception.h"
#include "emsg_mem.h"
#include "passphrase_provider.h"
#include "message_config.h"
#include "algo_spec.h"
#include "utility.h"

namespace EncryptMsg
{
    class MessageReaderImpl;

    class PacketAnalyzer : public NonCopyableNonMovable
    {
        public:
            PUBIF PacketAnalyzer();
            PUBIF ~PacketAnalyzer();

            // Start without passphrase to read headers before the data gets decrypted
            PUBIF void Start();

            PUBIF void Start(SafeVector passphrase);
            PUBIF void Start(std::unique_ptr<SafeVector> passphrase);
            PUBIF void Start(PassphraseProvider &passphrase_provider);

            // For known keys
            PUBIF void Start(SymmetricKeyProvider &key_provider);
            PUBIF void Start(EncryptionKey encryption_key);
            PUBIF void Start(std::unique_ptr<EncryptionKey> encryption_key);

            // Push buffer to analyze message
            // Returns true when the parameters are received
            // Throws EmsgException
            PUBIF bool Update(const SafeVector &buf);

            // Push buffer to analyze message and notify that the intput is complete
            // Returns true when the parameters are received
            // Throws EmsgException
            PUBIF bool Finish(const SafeVector &buf);

            PUBIF const EncryptionKey &GetEncryptionKey() const;
            PUBIF const MessageConfig &GetMessageConfig() const;
            PUBIF const Salt &GetSalt() const;

        private:
            MessageReaderImpl *impl_;
    };

    class MessageReader : public NonCopyableNonMovable
    {
        public:
            PUBIF MessageReader();
            PUBIF ~MessageReader();

            PUBIF void Start(SafeVector passphrase);
            PUBIF void Start(std::unique_ptr<SafeVector> passphrase);
            PUBIF void Start(PassphraseProvider &passphrase_provider);

            // For known keys
            PUBIF void Start(SymmetricKeyProvider &key_provider);
            PUBIF void Start(EncryptionKey encryption_key);
            PUBIF void Start(std::unique_ptr<EncryptionKey> encryption_key);

            PUBIF void Update(SafeVector &buf);
            PUBIF void Finish(SafeVector &buf);

            PUBIF const EncryptionKey &GetEncryptionKey() const;
            PUBIF const MessageConfig &GetMessageConfig() const;
            PUBIF const Salt &GetSalt() const;
        private:
            MessageReaderImpl *impl_;
    };

}

