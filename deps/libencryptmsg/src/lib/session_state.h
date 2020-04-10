//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <vector>
#include <stack>
#include <memory>
#include "emsg_mem.h"
#include "packet_parsers.h"
#include "packet_reader.h"
#include "emsg_utility.h"
#include "message_config.h"
#include "passphrase_provider.h"
#include "armor_reader.h"

namespace EncryptMsg
{
    using PacketChain = std::vector<PacketType>;

    // Session state will persist between calls to Update, Context will not
    struct SessionState : public NonCopyableNonMovable
    {
        MessageConfig message_config;
        Salt salt;
        PacketChain packet_chain;
        PacketChain::iterator packet_chain_it;
        SymmetricKeyProvider *key_provider;
        std::unique_ptr<EncryptionKey> encryption_key;
        ArmorReader armor_reader;

        // Stack of buffer that should be processed from top onwards
        // The invariant is that there are no empty buffers. If a buffer is empty, it should be popped.
        std::stack<SafeVector> buffer_stack;
        SafeVector output;
        PacketFactory packet_factory;
        EmsgResult emsg_result;
        bool is_message_analyzed;
        bool finish_packets;

        SessionState();
    };
}
