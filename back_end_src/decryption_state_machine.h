#pragma once
#include "emsg_types.h"
#include "packet_stream.h"
#include "packet_composer.h"
#include "decryption_state_machine.h"
#include "state_machine_interface.h"
#include "emsg_mem.h"
#include "message_decryption.h"
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
        LibEncryptMsg::MessageReader reader;
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

    class DecryptionContext : public LightStateMachine::StateMachineContext
    {
        private:
            using super = LightStateMachine::StateMachineContext;
            int filter_count_;
            Format format_;
            EpadResult result_;
            bool is_wad_head_finished_;
            InStream &in_;
            OutStream &out_;
            const EncryptParams &encrypt_params_;
            PacketMetadata &metadata_;
            LibEncryptMsg::SafeVector buffer_;
            LibEncryptMsg::SafeVector pending_buffer_;
            std::unique_ptr<DecryptionSession> passphrase_session_;
            std::unique_ptr<DecryptionSession> key_file_session_;
        public:
            DecryptionContext(InStream &in, OutStream &out,
                    const EncryptParams &encrypt_params, PacketMetadata &metadata):
                super(),
                filter_count_(0),
                format_(Format::Unknown),
                result_(EpadResult::None),
                is_wad_head_finished_(false),
                in_(in),
                out_(out),
                encrypt_params_(encrypt_params),
                metadata_(metadata)
            {
            }

            InStream &In()
            {
                return in_;
            }

            OutStream &Out()
            {
                return out_;
            }

            void SetFilterCount(int filter_count)
            {
                filter_count_ = filter_count;
            }

            int GetFilterCount() const
            {
                return filter_count_;
            }

            EpadResult GetResult() const
            {
                return result_;
            }

            void SetResult(EpadResult result)
            {
                result_ = result;
            }

            bool IsWADHeadFinished() const
            {
                return is_wad_head_finished_;
            }

            void SetWADHeadFinished(bool value)
            {
                is_wad_head_finished_ = value;
            }

            Format GetFormat() const
            {
                return format_;
            }

            void SetFormat(Format format)
            {
                format_ = format;
            }

            LibEncryptMsg::SafeVector &Buffer()
            {
                return buffer_;
            }

            LibEncryptMsg::SafeVector &PendingBuffer()
            {
                return pending_buffer_;
            }

            PacketMetadata &Metadata()
            {
                return metadata_;
            }

            const EncryptParams &GetEncryptParams() const
            {
                return encrypt_params_;
            }

            std::unique_ptr<DecryptionSession> &PassphraseSession()
            {
                return passphrase_session_;
            }

            std::unique_ptr<DecryptionSession> &KeyFileSession()
            {
                return key_file_session_;
            }
    };

    EpadResult DecryptStream(InStream &in, const EncryptParams &encrypt_params,
            OutStream &out, PacketMetadata &metadata);
}

