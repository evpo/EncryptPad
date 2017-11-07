#pragma once

#include <string>
#include "emsg_types.h"
#include "emsg_utility.h"
#include "emsg_exception.h"
#include "emsg_mem.h"
#include "message_config.h"

namespace LibEncryptMsg
{
    class MessageWriterImpl;
    class MessageWriter : public NonCopyableNonMovable
    {
        public:
            MessageWriter();
            ~MessageWriter();

            void Start(const SafeVector &passphrase, MessageConfig message_config = MessageConfig(),
                    Salt salt = Salt());
            void Start(std::unique_ptr<SafeVector> passphrase, MessageConfig message_config = MessageConfig(),
                    Salt salt = Salt());

            void Start(EncryptionKey encryption_key, MessageConfig message_config, Salt salt);
            void Start(std::unique_ptr<EncryptionKey> encryption_key, MessageConfig message_config, Salt salt);

            void Update(SafeVector& buf);
            void Finish(SafeVector& buf);

            const EncryptionKey &GetEncryptionKey() const;
            const Salt &GetSalt() const;
            const MessageConfig &GetMessageConfig() const;
        private:
            MessageWriterImpl *impl_;
    };
}
