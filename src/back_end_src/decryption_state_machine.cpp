#include "decryption_state_machine.h"
#include <limits>
#include "state_graph.h"
#include "state_machine.h"
#include "encryptmsg/emsg_exception.h"
#include "decryption_state_debug.h"
#include "x2_key_loader.h"
#include "key_file_converter.h"
#include "wad_reader_writer.h"
#include "epad_result.h"
#include "encryptmsg/openpgp_conversions.h"

using namespace LightStateMachine;
using namespace EncryptMsg;
using namespace EncryptPad;

namespace EncryptPad
{

    struct DecryptionStateMachine : public LightStateMachine::NonCopyable
    {
        int filter_count_;
        Format format_;
        EpadResult result_;
        bool is_wad_head_finished_;
        InStream &in_;
        OutStream &out_;
        const EncryptParams &encrypt_params_;
        PacketMetadata &metadata_;
        EncryptMsg::SafeVector buffer_;
        EncryptMsg::SafeVector pending_buffer_;
        std::unique_ptr<DecryptionSession> passphrase_session_;
        std::unique_ptr<DecryptionSession> key_file_session_;
        ProgressEvent progress_event_;

        StateGraph graph_;
        StateMachineContext context_;
        StateMachine state_machine_;

        bool ReadIn_CanEnter(StateMachineContext &ctx);
        bool ParseFormat_CanEnter(StateMachineContext &ctx);
        bool SetPwdKey_CanEnter(StateMachineContext &ctx);
        bool ReadKeyFile_CanEnter(StateMachineContext &ctx);
        bool GPG_CanEnter(StateMachineContext &ctx);
        bool WADHead_CanEnter(StateMachineContext &ctx);
        bool WriteOut_CanEnter(StateMachineContext &ctx);
        bool End_CanEnter(StateMachineContext &ctx);

        void ReadIn_OnEnter(StateMachineContext &ctx);
        void ParseFormat_OnEnter(StateMachineContext &ctx);
        void SetPwdKey_OnEnter(StateMachineContext &ctx);
        void ReadKeyFile_OnEnter(StateMachineContext &ctx);
        void GPG_OnEnter(StateMachineContext &ctx);
        void WADHead_OnEnter(StateMachineContext &ctx);
        void WriteOut_OnEnter(StateMachineContext &ctx);
        void Fail_OnEnter(StateMachineContext &ctx);
        void End_OnEnter(StateMachineContext &ctx);

        bool IsEncryptedEmptyString();

        DecryptionStateMachine(InStream &in, OutStream &out,
                const EncryptParams &encrypt_params, PacketMetadata &metadata);
        void DecryptStream();
    };

    DecryptionStateMachine::DecryptionStateMachine(InStream &in, OutStream &out,
            const EncryptParams &encrypt_params, PacketMetadata &metadata):
        filter_count_(0),
        format_(Format::Unknown),
        result_(EpadResult::None),
        is_wad_head_finished_(false),
        in_(in),
        out_(out),
        encrypt_params_(encrypt_params),
        metadata_(metadata),
        progress_event_(in_.GetCount(), 0),
        state_machine_(graph_, context_)
    {
        using Self = DecryptionStateMachine;
        using VoidF = LightStateMachine::VoidMemberFunction<DecryptionStateMachine>;
        using BoolF = LightStateMachine::BoolMemberFunction<DecryptionStateMachine>;

        // Start state

        graph_.Create(StateID::Start);

        // Fail state

        graph_.Create(StateID::Fail,
                VoidF(this, &Self::Fail_OnEnter), StubVoidFunction,
                StubBoolFunction, AlwaysFalseBoolFunction);

        graph_.SetStartStateID(StateID::Start);
        graph_.SetFailStateID(StateID::Fail);

        // Other states

        graph_.Create(StateID::ReadIn,
                VoidF(this, &Self::ReadIn_OnEnter), StubVoidFunction,
                BoolF(this, &Self::ReadIn_CanEnter), StubBoolFunction);

        graph_.Create(StateID::ParseFormat,
                VoidF(this, &Self::ParseFormat_OnEnter), StubVoidFunction,
                BoolF(this, &Self::ParseFormat_CanEnter), StubBoolFunction);

        graph_.Create(StateID::SetPwdKey,
                VoidF(this, &Self::SetPwdKey_OnEnter), StubVoidFunction,
                BoolF(this, &Self::SetPwdKey_CanEnter), StubBoolFunction);

        graph_.Create(StateID::ReadKeyFile,
                VoidF(this, &Self::ReadKeyFile_OnEnter), StubVoidFunction,
                BoolF(this, &Self::ReadKeyFile_CanEnter), StubBoolFunction);

        graph_.Create(StateID::GPG,
                VoidF(this, &Self::GPG_OnEnter), StubVoidFunction,
                BoolF(this, &Self::GPG_CanEnter), StubBoolFunction);

        graph_.Create(StateID::WADHead,
                VoidF(this, &Self::WADHead_OnEnter), StubVoidFunction,
                BoolF(this, &Self::WADHead_CanEnter), StubBoolFunction);

        graph_.Create(StateID::WriteOut,
                VoidF(this, &Self::WriteOut_OnEnter), StubVoidFunction,
                BoolF(this, &Self::WriteOut_CanEnter), StubBoolFunction);

        graph_.Create(StateID::End,
                VoidF(this, &Self::End_OnEnter), StubVoidFunction,
                BoolF(this, &Self::End_CanEnter), AlwaysFalseBoolFunction);


        graph_.Link(StateID::Start, StateID::ReadIn);
        graph_.Link(StateID::ReadIn, StateID::End);
        graph_.Link(StateID::ReadIn, StateID::ParseFormat);
        graph_.Link(StateID::ReadIn, StateID::GPG);
        graph_.Link(StateID::ReadIn, StateID::WADHead);

        graph_.Link(StateID::ParseFormat, StateID::ReadIn);
        graph_.Link(StateID::ParseFormat, StateID::GPG);
        graph_.Link(StateID::ParseFormat, StateID::WADHead);
        graph_.Link(StateID::ParseFormat, StateID::ReadKeyFile);
        graph_.Link(StateID::ParseFormat, StateID::SetPwdKey);
        graph_.Link(StateID::ParseFormat, StateID::WriteOut);

        graph_.Link(StateID::SetPwdKey, StateID::GPG);

        graph_.Link(StateID::ReadKeyFile, StateID::ReadIn);
        graph_.Link(StateID::ReadKeyFile, StateID::GPG);

        graph_.Link(StateID::GPG, StateID::ParseFormat);
        graph_.Link(StateID::GPG, StateID::ReadIn);
        graph_.Link(StateID::GPG, StateID::WADHead);
        graph_.Link(StateID::GPG, StateID::GPG);
        graph_.Link(StateID::GPG, StateID::WriteOut);

        graph_.Link(StateID::WADHead, StateID::ReadKeyFile);
        graph_.Link(StateID::WADHead, StateID::GPG);
        graph_.Link(StateID::WADHead, StateID::ReadIn);

        graph_.Link(StateID::WriteOut, StateID::ReadIn);

        state_machine_.SetStateIDToStringConverter(
                [](StateMachineStateID id){
                    return std::string("decryption: ") +
                    PrintDecryptionStateMachineStateID(id);});
    }

    void DecryptionStateMachine::DecryptStream()
    {
        while(state_machine_.NextState())
        {
        }
    }

    EpadResult DecryptStream(InStream &in, const EncryptParams &encrypt_params,
            OutStream &out, PacketMetadata &metadata)
    {
        DecryptionStateMachine state_machine(in, out, encrypt_params, metadata);
        state_machine.DecryptStream();
        return state_machine.result_;
    }

    bool DecryptionStateMachine::ReadIn_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        return buffer_.size() == 0;
    }

    void DecryptionStateMachine::ReadIn_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        //memory_buffer should be smaller than max sizes of both types
        assert(static_cast<uint64_t>(encrypt_params_.memory_buffer) <=
                std::min(
                    static_cast<uint64_t>(std::numeric_limits<stream_length_type>::max()),
                    static_cast<uint64_t>(std::numeric_limits<size_t>::max())
                    ));
        stream_length_type size2read = std::min(static_cast<stream_length_type>(encrypt_params_.memory_buffer), in_.GetCount());
        buffer_.resize(static_cast<size_t>(size2read));
        size_t size = in_.Read(buffer_.data(), buffer_.size());
        buffer_.resize(size);
        filter_count_ = 0;
        encrypt_params_.progress_callback(progress_event_);
        if(progress_event_.cancel)
        {
            result_ = EpadResult::Cancelled;
            ctx.SetFailed(true);
        }
        progress_event_.complete_bytes += size;

    }

    bool DecryptionStateMachine::End_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        return in_.IsEOF() && buffer_.empty() && pending_buffer_.empty()
            && result_ == EpadResult::Success;
    }

    void DecryptionStateMachine::End_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        // Take outer reader
        MessageReader *reader = nullptr;
        switch(format_)
        {
            case Format::NestedWAD:
            case Format::GPG:
                reader = &passphrase_session_->reader;
                break;

            case Format::GPGByKeyFile:
            case Format::WAD:
                reader = &key_file_session_->reader;
                break;

            default:
                result_ = EpadResult::UnexpectedError;
                ctx.SetFailed(true);
                return;
        }
        auto &metadata = metadata_;
        auto &config = reader->GetMessageConfig();

        metadata.file_name = config.GetFileName();
        metadata.file_date = config.GetFileDate();
        metadata.is_binary = config.GetBinary();
        metadata.cipher_algo = config.GetCipherAlgo();
        metadata.compression = config.GetCompression();
        metadata.hash_algo = config.GetHashAlgo();
        metadata.iterations = DecodeS2KIterations(config.GetIterations());
        metadata.salt = reader->GetSalt();

        result_ = EpadResult::Success;
    }

    bool DecryptionStateMachine::ParseFormat_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        switch(format_)
        {
            case Format::Unknown:
                break;
            case Format::GPGOrNestedWad:
                if(filter_count_ != 1)
                    return false;
                break;
            default:
                return false;
        }

        return buffer_.size() > 0 || pending_buffer_.size() > 0 || IsEncryptedEmptyString();
    }

    void DecryptionStateMachine::ParseFormat_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        size_t required_bytes = filter_count_ == 1 ? 4 : 1;

        if(IsEncryptedEmptyString())
        {
            format_ = Format::GPG;
            return;
        }

        pending_buffer_.insert(pending_buffer_.end(), buffer_.begin(), buffer_.end());
        buffer_.clear();

        // We need more bytes
        if(pending_buffer_.size() < required_bytes && !in_.IsEOF())
            return;

        if(filter_count_ == 0)
        {
            uint8_t b = pending_buffer_[0];
            if((b & 0x80 && b != 0xEF) || b == '-')
            {
                if(metadata_.key_only)
                {
                    format_ = Format::GPGByKeyFile;
                }
                else
                {
                    format_ = Format::GPGOrNestedWad;
                }
            }
            else // wad starts from I or P, in which the most significant bit is not set
            {
                format_ = Format::WAD;
            }
        }
        else if(filter_count_ == 1)
        {
            std::string marker;

            if(pending_buffer_.size() >= 4)
            {
                marker.insert(0U, reinterpret_cast<const char*>(pending_buffer_.data()), 4U);
            }

            if(marker == "IWAD" || marker == "PWAD")
            {
                format_ = Format::NestedWAD;
            }
            else
            {
                format_ = Format::GPG;
            }
        }
        else
        {
            // Filter count can be only 0 or 1
            assert(false);
        }

        buffer_.swap(pending_buffer_);
    }

    bool DecryptionStateMachine::GPG_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        if(filter_count_ > 1)
            return false;

        switch(format_)
        {
            case Format::Empty:
            case Format::Unknown:
                return false;

            case Format::GPG:
            case Format::GPGOrNestedWad:
                if(!passphrase_session_)
                    return false;
                if(filter_count_ == 1)
                    return false;
                break;

            case Format::GPGByKeyFile:
                if(!key_file_session_)
                    return false;
                if(filter_count_ == 1)
                    return false;
                break;

            case Format::WAD:
                if(!is_wad_head_finished_)
                    return false;
                if(!key_file_session_)
                    return false;
                if(filter_count_ == 1)
                    return false;
                break;

            case Format::NestedWAD:
                assert(passphrase_session_);
                if(filter_count_ == 1 && !is_wad_head_finished_)
                    return false;
                if(filter_count_ == 1 && !key_file_session_)
                    return false;
                break;

            default:
                break;

        }

        if(buffer_.size() == 0)
            return false;

        return true;
    }

    void DecryptionStateMachine::GPG_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        using namespace EncryptMsg;
        MessageReader *reader = nullptr;
        bool is_key_file_session = false;
        switch(format_)
        {
            case Format::GPG:
            case Format::GPGOrNestedWad:
                assert(filter_count_ == 0);
                reader = &passphrase_session_->reader;
                break;

            case Format::NestedWAD:
                is_key_file_session = (filter_count_ > 0);
                reader = (!is_key_file_session)
                    ?
                    &passphrase_session_->reader
                    :
                    &key_file_session_->reader;
                break;

            default:
                is_key_file_session = true;
                reader = &key_file_session_->reader;
                break;
        }

        try
        {
            if(in_.IsEOF())
            {
                reader->Finish(buffer_);
            }
            else
            {
                reader->Update(buffer_);
            }
        }
        catch(const EmsgException &e)
        {
            result_ = ToEpadResult(e.result);
            if(is_key_file_session && (result_ == EpadResult::InvalidSurrogateIV ||
                        result_ == EpadResult::MDCError))
            {
                result_ = EpadResult::InvalidKeyFile;
            }
            ctx.SetFailed(true);
            return;
        }
        filter_count_ ++;
        result_ = EpadResult::Success;
    }

    bool DecryptionStateMachine::SetPwdKey_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        switch(format_)
        {
            case Format::GPG:
            case Format::GPGOrNestedWad:
                if(!passphrase_session_)
                    return true;
                break;

            default:
                break;
        }
        return false;
    }

    void DecryptionStateMachine::SetPwdKey_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        passphrase_session_.reset(
                new DecryptionSession(
                    encrypt_params_.key_service,
                    encrypt_params_.passphrase)
                );
    }

    bool DecryptionStateMachine::WriteOut_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        if(buffer_.size() == 0)
            return false;

        switch(format_)
        {
            case Format::Empty:
            case Format::Unknown:
            case Format::GPGOrNestedWad:
                return false;

            case Format::GPGByKeyFile:
            case Format::GPG:
            case Format::WAD:
                if(filter_count_ != 1)
                    return false;
                break;

            case Format::NestedWAD:
                if(filter_count_ != 2)
                    return false;
                break;

            default:
                break;
        }
        return true;
    }

    void DecryptionStateMachine::WriteOut_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        out_.Write(buffer_.data(), buffer_.size());
        buffer_.clear();
    }

    void DecryptionStateMachine::Fail_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        // If we came here because no states can enter
        if(result_ == EpadResult::Success)
            result_ = EpadResult::UnexpectedError;
    }

    bool DecryptionStateMachine::ReadKeyFile_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        if(key_file_session_)
            return false;

        switch(format_)
        {
            case Format::GPGByKeyFile:
                break;

            case Format::WAD:
            case Format::NestedWAD:
                if(!is_wad_head_finished_)
                    return false;
                break;

            default:
                return false;
        }
        return true;
    }

    void DecryptionStateMachine::ReadKeyFile_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        PacketMetadata &metadata = metadata_;

        if(metadata.key_file.empty())
        {
            result_ = EpadResult::KeyFileNotSpecified;
            ctx.SetFailed(true);
            return;
        }

        key_file_session_.reset(new DecryptionSession());

        std::string empty_str;
        EpadResult result = LoadKeyFromFile(metadata.key_file,
                encrypt_params_.libcurl_path ? *encrypt_params_.libcurl_path : empty_str,
                encrypt_params_.libcurl_parameters ? *encrypt_params_.libcurl_parameters : empty_str,
                key_file_session_->own_passphrase);

        if(result != EpadResult::Success)
        {
            result_ = result;
            ctx.SetFailed(true);
            return;
        }

        if(!DecryptKeyFileContent(key_file_session_->own_passphrase,
                    encrypt_params_.key_file_encrypt_params, key_file_session_->own_passphrase))
        {
            result_ = EpadResult::InvalidKeyFilePassphrase;
            ctx.SetFailed(true);
            return;
        }

        if(!ValidateDecryptedKeyFile(key_file_session_->own_passphrase))
        {
            result_ = EpadResult::InvalidKeyFile;
            ctx.SetFailed(true);
            return;
        }
        result_ = EpadResult::Success;
    }

    bool DecryptionStateMachine::WADHead_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        if(is_wad_head_finished_)
            return false;

        if(buffer_.size() == 0 && pending_buffer_.size() == 0)
            return false;

        switch(format_)
        {
            case Format::WAD:
            case Format::NestedWAD:
                break;

            default:
                return false;
        }
        return true;
    }

    void DecryptionStateMachine::WADHead_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        pending_buffer_.insert(pending_buffer_.end(), buffer_.begin(), buffer_.end());
        buffer_.clear();

        InPacketStreamMemory stm_in(pending_buffer_.data(),
                pending_buffer_.data() + pending_buffer_.size());

        uint32_t payload_offset = 0;
        uint32_t payload_size = 0;
        std::string key_file;
        EpadResult result = ParseWad(stm_in, key_file, payload_offset, payload_size);

        switch(result)
        {
            case EpadResult::Success:
                break;

            case EpadResult::InvalidOrIncompleteWadFile:
                if(in_.IsEOF())
                {
                    result_ = result;
                    ctx.SetFailed(true);
                }
                return;

            default:
                result_ = result;
                ctx.SetFailed(true);
                return;
        }

        if(metadata_.key_file.empty())
            metadata_.key_file = key_file;

        buffer_.swap(pending_buffer_);
        buffer_.erase(buffer_.begin(), buffer_.begin() + payload_offset);
        if(payload_size != 0 && payload_size < buffer_.size())
        {
            //This is 3.2.1 format, in which the key string and the dictionary goes after the payload.
            //We need to trim the file
            buffer_.erase(buffer_.begin() + payload_size, buffer_.end());
        }
        is_wad_head_finished_ = true;
        result_ = EpadResult::Success;
    }

    bool DecryptionStateMachine::IsEncryptedEmptyString()
    {
        return in_.IsEOF() && filter_count_ == 1 && buffer_.empty() && pending_buffer_.empty()
            && format_ == Format::GPGOrNestedWad;
    }
}

