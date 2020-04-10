#pragma once
#include "encryptmsg/emsg_types.h"
#include "packet_stream.h"
#include "packet_composer.h"
#include "decryption_state_machine.h"
#include "state_machine_interface.h"
#include "encryptmsg/emsg_mem.h"
#include "encryptmsg/message_decryption.h"
#include "key_service_key_provider.h"
#include <string>

namespace EncryptPad
{
    enum class StateID : LightStateMachine::StateMachineStateID
    {
        Start,
        End,
        ReadIn,
        ParseFormat,
        SetPwdKey,
        ReadKeyFile,
        GPG,
        WADHead,
        WriteOut,
        Fail,
    };

    enum class Format
    {
        Unknown,
        GPG,
        GPGByKeyFile,
        WAD,
        NestedWAD,
        GPGOrNestedWad,
        Empty,
    };

    struct DecryptionSession
    {
        EncryptMsg::MessageReader reader;
        KeyServiceKeyProvider key_provider;

        //These are only for memory management
        std::string own_passphrase;
        KeyService own_key_service;

        DecryptionSession()
            :key_provider(&own_key_service, &own_passphrase)
        {
            reader.Start(key_provider);
        }

        DecryptionSession(KeyService *key_service, const std::string *passphrase)
            :key_provider(key_service, passphrase)
        {
            reader.Start(key_provider);
        }
    };

    EpadResult DecryptStream(InStream &in, const EncryptParams &encrypt_params,
            OutStream &out, PacketMetadata &metadata);
}

