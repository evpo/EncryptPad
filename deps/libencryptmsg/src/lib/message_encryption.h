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
#include "message_config.h"
#include "utility.h"

namespace EncryptMsg
{
    class MessageWriterImpl;
    class MessageWriter : public NonCopyableNonMovable
    {
        public:
            PUBIF MessageWriter();
            PUBIF ~MessageWriter();

            PUBIF void Start(const SafeVector &passphrase, MessageConfig message_config = MessageConfig(),
                    Salt salt = Salt());
            PUBIF void Start(std::unique_ptr<SafeVector> passphrase, MessageConfig message_config = MessageConfig(),
                    Salt salt = Salt());

            PUBIF void Start(EncryptionKey encryption_key, MessageConfig message_config, Salt salt);
            PUBIF void Start(std::unique_ptr<EncryptionKey> encryption_key, MessageConfig message_config, Salt salt);

            PUBIF void Update(SafeVector& buf);
            PUBIF void Finish(SafeVector& buf);

            PUBIF const EncryptionKey &GetEncryptionKey() const;
            PUBIF const Salt &GetSalt() const;
            PUBIF const MessageConfig &GetMessageConfig() const;
        private:
            MessageWriterImpl *impl_;
    };
}
