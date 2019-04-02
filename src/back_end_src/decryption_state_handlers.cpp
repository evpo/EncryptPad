#include "decryption_state_handlers.h"
#include "decryption_state_machine.h"
#include "x2_key_loader.h"
#include "key_file_converter.h"
#include "wad_reader_writer.h"
#include "epad_result.h"
#include "encryptmsg/openpgp_conversions.h"

using namespace EncryptMsg;
using namespace EncryptPad;

namespace
{
    inline DecryptionContext &ToContext(LightStateMachine::StateMachineContext &ctx)
    {
        return *static_cast<DecryptionContext*>(&ctx);
    }

    inline bool IsEncryptedEmptyString(DecryptionContext &c)
    {
        return c.In().IsEOF() && c.GetFilterCount() == 1 && c.Buffer().empty() && c.PendingBuffer().empty()
            && c.GetFormat() == Format::GPGOrNestedWad;
    }

}

namespace EncryptPad
{
    bool ReadIn_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        return c.Buffer().size() == 0;
    }

    void ReadIn_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        //memory_buffer should be smaller than max sizes of both types
        assert(static_cast<uint64_t>(c.GetEncryptParams().memory_buffer) <=
                std::min(
                    static_cast<uint64_t>(std::numeric_limits<stream_length_type>::max()),
                    static_cast<uint64_t>(std::numeric_limits<size_t>::max())
                    ));
        stream_length_type size2read = std::min(static_cast<stream_length_type>(c.GetEncryptParams().memory_buffer), c.In().GetCount());
        c.Buffer().resize(static_cast<size_t>(size2read));
        size_t size = c.In().Read(c.Buffer().data(), c.Buffer().size());
        c.Buffer().resize(size);
        c.SetFilterCount(0);
        c.GetEncryptParams().progress_callback(c.GetProgressEvent());
        if(c.GetProgressEvent().cancel)
        {
            c.SetResult(EpadResult::Cancelled);
            c.SetFailed(true);
        }
        c.GetProgressEvent().complete_bytes += size;

    }

    bool End_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        return c.In().IsEOF() && c.Buffer().empty() && c.PendingBuffer().empty()
            && c.GetResult() == EpadResult::Success;
    }

    void End_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        // Take outer reader
        MessageReader *reader = nullptr;
        switch(c.GetFormat())
        {
            case Format::NestedWAD:
            case Format::GPG:
                reader = &c.PassphraseSession()->reader;
                break;

            case Format::GPGByKeyFile:
            case Format::WAD:
                reader = &c.KeyFileSession()->reader;
                break;

            default:
                c.SetResult(EpadResult::UnexpectedError);
                c.SetFailed(true);
                return;
        }
        auto &metadata = c.Metadata();
        auto &config = reader->GetMessageConfig();

        metadata.file_name = config.GetFileName();
        metadata.file_date = config.GetFileDate();
        metadata.is_binary = config.GetBinary();
        metadata.cipher_algo = config.GetCipherAlgo();
        metadata.compression = config.GetCompression();
        metadata.hash_algo = config.GetHashAlgo();
        metadata.iterations = DecodeS2KIterations(config.GetIterations());
        metadata.salt = reader->GetSalt();

        c.SetResult(EpadResult::Success);
    }

    bool ParseFormat_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        switch(c.GetFormat())
        {
            case Format::Unknown:
                break;
            case Format::GPGOrNestedWad:
                if(c.GetFilterCount() != 1)
                    return false;
                break;
            default:
                return false;
        }

        return c.Buffer().size() > 0 || c.PendingBuffer().size() > 0 || IsEncryptedEmptyString(c);
    }

    void ParseFormat_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        size_t required_bytes = c.GetFilterCount() == 1 ? 4 : 1;

        if(IsEncryptedEmptyString(c))
        {
            c.SetFormat(Format::GPG);
            return;
        }

        c.PendingBuffer().insert(c.PendingBuffer().end(), c.Buffer().begin(), c.Buffer().end());
        c.Buffer().clear();

        // We need more bytes
        if(c.PendingBuffer().size() < required_bytes && !c.In().IsEOF())
            return;

        if(c.GetFilterCount() == 0)
        {
            uint8_t b = c.PendingBuffer()[0];
            if(b & 0x80 && b != 0xEF)
            {
                if(c.Metadata().key_only)
                {
                    c.SetFormat(Format::GPGByKeyFile);
                }
                else
                {
                    c.SetFormat(Format::GPGOrNestedWad);
                }
            }
            else // wad starts from I or P, in which the most significant bit is not set
            {
                c.SetFormat(Format::WAD);
            }
        }
        else if(c.GetFilterCount() == 1)
        {
            std::string marker;

            if(c.PendingBuffer().size() >= 4)
            {
                marker.insert(0U, reinterpret_cast<const char*>(c.PendingBuffer().data()), 4U);
            }

            if(marker == "IWAD" || marker == "PWAD")
            {
                c.SetFormat(Format::NestedWAD);
            }
            else
            {
                c.SetFormat(Format::GPG);
            }
        }
        else
        {
            // Filter count can be only 0 or 1
            assert(false);
        }

        c.Buffer().swap(c.PendingBuffer());
    }

    bool GPG_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        if(c.GetFilterCount() > 1)
            return false;

        switch(c.GetFormat())
        {
            case Format::Empty:
            case Format::Unknown:
                return false;

            case Format::GPG:
            case Format::GPGOrNestedWad:
                if(!c.PassphraseSession())
                    return false;
                if(c.GetFilterCount() == 1)
                    return false;
                break;

            case Format::GPGByKeyFile:
                if(!c.KeyFileSession())
                    return false;
                if(c.GetFilterCount() == 1)
                    return false;
                break;

            case Format::WAD:
                if(!c.IsWADHeadFinished())
                    return false;
                if(!c.KeyFileSession())
                    return false;
                if(c.GetFilterCount() == 1)
                    return false;
                break;

            case Format::NestedWAD:
                assert(c.PassphraseSession());
                if(c.GetFilterCount() == 1 && !c.IsWADHeadFinished())
                    return false;
                if(c.GetFilterCount() == 1 && !c.KeyFileSession())
                    return false;
                break;

            default:
                break;

        }

        if(c.Buffer().size() == 0)
            return false;

        return true;
    }

    void GPG_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        using namespace EncryptMsg;
        auto &c = ToContext(ctx);
        MessageReader *reader = nullptr;
        bool is_key_file_session = false;
        switch(c.GetFormat())
        {
            case Format::GPG:
            case Format::GPGOrNestedWad:
                assert(c.GetFilterCount() == 0);
                reader = &c.PassphraseSession()->reader;
                break;

            case Format::NestedWAD:
                is_key_file_session = (c.GetFilterCount() > 0);
                reader = (!is_key_file_session)
                    ?
                    &c.PassphraseSession()->reader
                    :
                    &c.KeyFileSession()->reader;
                break;

            default:
                is_key_file_session = true;
                reader = &c.KeyFileSession()->reader;
                break;
        }

        try
        {
            if(c.In().IsEOF())
            {
                reader->Finish(c.Buffer());
            }
            else
            {
                reader->Update(c.Buffer());
            }
        }
        catch(const EmsgException &e)
        {
            c.SetResult(ToEpadResult(e.result));
            if(is_key_file_session && (c.GetResult() == EpadResult::InvalidSurrogateIV ||
                        c.GetResult() == EpadResult::MDCError))
            {
                c.SetResult(EpadResult::InvalidKeyFile);
            }
            c.SetFailed(true);
            return;
        }
        c.SetFilterCount(c.GetFilterCount() + 1);
        c.SetResult(EpadResult::Success);
    }

    bool SetPwdKey_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        switch(c.GetFormat())
        {
            case Format::GPG:
            case Format::GPGOrNestedWad:
                if(!c.PassphraseSession())
                    return true;
                break;

            default:
                break;
        }
        return false;
    }

    void SetPwdKey_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        c.PassphraseSession().reset(
                new DecryptionSession(
                    c.GetEncryptParams().key_service,
                    c.GetEncryptParams().passphrase)
                );
    }
    bool WriteOut_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        if(c.Buffer().size() == 0)
            return false;

        switch(c.GetFormat())
        {
            case Format::Empty:
            case Format::Unknown:
            case Format::GPGOrNestedWad:
                return false;

            case Format::GPGByKeyFile:
            case Format::GPG:
            case Format::WAD:
                if(c.GetFilterCount() != 1)
                    return false;
                break;

            case Format::NestedWAD:
                if(c.GetFilterCount() != 2)
                    return false;
                break;

            default:
                break;
        }
        return true;
    }

    void WriteOut_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        c.Out().Write(c.Buffer().data(), c.Buffer().size());
        c.Buffer().clear();
    }

    void Fail_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        // If we came here because no states can enter
        if(c.GetResult() == EpadResult::Success)
            c.SetResult(EpadResult::UnexpectedError);
    }

    bool ReadKeyFile_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        if(c.KeyFileSession())
            return false;

        switch(c.GetFormat())
        {
            case Format::GPGByKeyFile:
                break;

            case Format::WAD:
            case Format::NestedWAD:
                if(!c.IsWADHeadFinished())
                    return false;
                break;

            default:
                return false;
        }
        return true;
    }

    void ReadKeyFile_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        PacketMetadata &metadata = c.Metadata();
        const EncryptParams &encrypt_params = c.GetEncryptParams();

        if(metadata.key_file.empty())
        {
            c.SetResult(EpadResult::KeyFileNotSpecified);
            c.SetFailed(true);
            return;
        }

        c.KeyFileSession().reset(new DecryptionSession());

        std::string empty_str;
        EpadResult result = LoadKeyFromFile(metadata.key_file,
                encrypt_params.libcurl_path ? *encrypt_params.libcurl_path : empty_str,
                encrypt_params.libcurl_parameters ? *encrypt_params.libcurl_parameters : empty_str,
                c.KeyFileSession()->own_passphrase);

        if(result != EpadResult::Success)
        {
            c.SetResult(result);
            c.SetFailed(true);
            return;
        }

        if(!DecryptKeyFileContent(c.KeyFileSession()->own_passphrase,
                    encrypt_params.key_file_encrypt_params, c.KeyFileSession()->own_passphrase))
        {
            c.SetResult(EpadResult::InvalidKeyFilePassphrase);
            c.SetFailed(true);
            return;
        }

        if(!ValidateDecryptedKeyFile(c.KeyFileSession()->own_passphrase))
        {
            c.SetResult(EpadResult::InvalidKeyFile);
            c.SetFailed(true);
            return;
        }
        c.SetResult(EpadResult::Success);
    }

    bool WADHead_CanEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);

        if(c.IsWADHeadFinished())
            return false;

        if(c.Buffer().size() == 0 && c.PendingBuffer().size() == 0)
            return false;

        switch(c.GetFormat())
        {
            case Format::WAD:
            case Format::NestedWAD:
                break;

            default:
                return false;
        }
        return true;
    }

    void WADHead_OnEnter(LightStateMachine::StateMachineContext &ctx)
    {
        auto &c = ToContext(ctx);
        c.PendingBuffer().insert(c.PendingBuffer().end(), c.Buffer().begin(), c.Buffer().end());
        c.Buffer().clear();

        InPacketStreamMemory stm_in(c.PendingBuffer().data(),
                c.PendingBuffer().data() + c.PendingBuffer().size());

        uint32_t payload_offset = 0;
        uint32_t payload_size = 0;
        std::string key_file;
        EpadResult result = ParseWad(stm_in, key_file, payload_offset, payload_size);

        switch(result)
        {
            case EpadResult::Success:
                break;

            case EpadResult::InvalidOrIncompleteWadFile:
                if(c.In().IsEOF())
                {
                    c.SetResult(result);
                    c.SetFailed(true);
                }
                return;

            default:
                c.SetResult(result);
                c.SetFailed(true);
                return;
        }

        if(c.Metadata().key_file.empty())
            c.Metadata().key_file = key_file;

        c.Buffer().swap(c.PendingBuffer());
        c.Buffer().erase(c.Buffer().begin(), c.Buffer().begin() + payload_offset);
        if(payload_size != 0 && payload_size < c.Buffer().size())
        {
            //This is 3.2.1 format, in which the key string and the dictionary goes after the payload.
            //We need to trim the file
            c.Buffer().erase(c.Buffer().begin() + payload_size, c.Buffer().end());
        }
        c.SetWADHeadFinished(true);
        c.SetResult(EpadResult::Success);
    }
}

