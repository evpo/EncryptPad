#pragma once

#include <string>
#include "emsg_types.h"
#include "emsg_utility.h"
#include "emsg_exception.h"
#include "emsg_mem.h"
#include "passphrase_provider.h"
#include "message_config.h"
#include "algo_spec.h"

namespace LibEncryptMsg
{
    class MessageReaderImpl;

    class PacketAnalyzer : public NonCopyableNonMovable
    {
        public:
            PacketAnalyzer();
            ~PacketAnalyzer();

            // Start without passphrase to read headers before the data gets decrypted
            void Start();

            void Start(SecureVector passphrase);
            void Start(std::unique_ptr<SecureVector> passphrase);
            void Start(PassphraseProvider &passphrase_provider);

            // For known keys
            void Start(SymmetricKeyProvider &key_provider);
            void Start(EncryptionKey encryption_key);
            void Start(std::unique_ptr<EncryptionKey> encryption_key);

            // Push buffer to analyze message
            // Returns true when the parameters are received
            // Throws EmsgException
            bool Update(const SecureVector &buf);

            // Push buffer to analyze message and notify that the intput is complete
            // Returns true when the parameters are received
            // Throws EmsgException
            bool Finish(const SecureVector &buf);

            const EncryptionKey &GetEncryptionKey() const;
            const MessageConfig &GetMessageConfig() const;
            const Salt &GetSalt() const;

        private:
            MessageReaderImpl *impl_;
    };

    class MessageReader : public NonCopyableNonMovable
    {
        public:
            MessageReader();
            ~MessageReader();

            void Start(SecureVector passphrase);
            void Start(std::unique_ptr<SecureVector> passphrase);
            void Start(PassphraseProvider &passphrase_provider);

            // For known keys
            void Start(SymmetricKeyProvider &key_provider);
            void Start(EncryptionKey encryption_key);
            void Start(std::unique_ptr<EncryptionKey> encryption_key);

            void Update(SecureVector &buf);
            void Finish(SecureVector &buf);

            const EncryptionKey &GetEncryptionKey() const;
            const MessageConfig &GetMessageConfig() const;
            const Salt &GetSalt() const;
        private:
            MessageReaderImpl *impl_;
    };

}

