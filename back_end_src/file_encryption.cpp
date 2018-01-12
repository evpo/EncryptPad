//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//This file is part of EncryptPad
//
//EncryptPad is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 2 of the License, or
//(at your option) any later version.
//
//EncryptPad is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EncryptPad.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************
#include "file_encryption.h"
#include "file_system.hpp"
#include <fstream>
#include <limits>
#include "packet_composer.h"
#include "wad_reader_writer.h"
#include "x2_key_loader.h"
#include "epad_utilities.h"
#include "file_helper.h"
#include "win_file_reader.h"
#include "key_file_converter.h"
#include "message_encryption.h"
#include "message_decryption.h"
#include "openpgp_conversions.h"
#include "emsg_symmetric_key.h"
#include "key_service_key_provider.h"
#include "decryption_state_machine.h"
#include "epad_result.h"

using namespace LibEncryptMsg;
namespace
{
    using namespace EncryptPad;
    typedef Botan::SecureVector<byte> Buffer;

    MessageConfig ConvertToMessageConfig(const PacketMetadata &metadata)
    {
        MessageConfig config;
        config.SetCipherAlgo(metadata.cipher_algo);
        config.SetHashAlgo(metadata.hash_algo);
        config.SetIterations(EncodeS2KIterations(metadata.iterations));
        config.SetCompression(metadata.compression);
        config.SetFileName(metadata.file_name);
        config.SetFileDate(metadata.file_date);
        config.SetBinary(metadata.is_binary);
        //TODO: use some meaningful default
        config.SetPartialLengthPower(16);
        return config;
    }

    struct SymmetricEncryptionSession
    {
        std::unique_ptr<EncryptionKey> key;
        Salt salt;
        EpadResult preparation_result;
        SymmetricEncryptionSession():preparation_result(EpadResult::None)
        {
        }

        bool IsValid()
        {
            return preparation_result == EpadResult::Success;
        }
    };

    SymmetricEncryptionSession PreparePassphraseSession(EncryptParams &encrypt_params, const MessageConfig &config)
    {
        SymmetricEncryptionSession ret_val;
        if(encrypt_params.passphrase)
        {
            Passphrase passphrase(SafeVector(encrypt_params.passphrase->cbegin(), encrypt_params.passphrase->cend()));
            ret_val.salt = GenerateRandomSalt();
            ret_val.key = GenerateEncryptionKey(passphrase, config.GetCipherAlgo(),
                    config.GetHashAlgo(), config.GetIterations(), ret_val.salt);
            ret_val.preparation_result = EpadResult::Success;
        }
        else
        {
            const KeyRecord &key_record = encrypt_params.key_service->GetKeyForSaving();
            ret_val.salt = key_record.salt;
            // We assume that key_service has been set with the same encryption parameters
            ret_val.key.reset(new EncryptionKey(key_record.key->bits_of()));
            ret_val.preparation_result = EpadResult::Success;
        }
        return ret_val;
    }

    SymmetricEncryptionSession PrepareKeyFileSession(EncryptParams &encrypt_params, const PacketMetadata &metadata,
            const MessageConfig &config)
    {
        SymmetricEncryptionSession session;
        std::string raw_key_phrase, key_phrase;
        std::string empty_str;
        session.preparation_result = LoadKeyFromFile(metadata.key_file,
                encrypt_params.libcurl_path ? *encrypt_params.libcurl_path : empty_str,
                encrypt_params.libcurl_parameters ? *encrypt_params.libcurl_parameters : empty_str,
                raw_key_phrase);

        if(session.preparation_result != EpadResult::Success)
            return session;

        //TODO: key_phrase should be SafeVector
        if(!DecryptKeyFileContent(raw_key_phrase, encrypt_params.key_file_encrypt_params, key_phrase))
        {
            session.preparation_result = EpadResult::InvalidKeyFilePassphrase;
            return session;
        }
        Passphrase passphrase(SafeVector(key_phrase.cbegin(), key_phrase.cend()));
        std::fill(key_phrase.begin(), key_phrase.end(), '0');
        session.salt = GenerateRandomSalt();
        session.key = GenerateEncryptionKey(passphrase, config.GetCipherAlgo(),
                config.GetHashAlgo(), config.GetIterations(), session.salt);
        session.preparation_result = EpadResult::Success;
        return session;
    }

    enum class FileNestingMode
    {
        Unknown,
        SimpleGPG,
        SimpleGPGWithKey,
        WadWithGPG,
        NestedGPGWithWad,
        SimpleGPGOrNestedGPGWithWad, //This mode is only for decryption when we don't know what the encrypted file contains yet
    };

    FileNestingMode IdentifyFileNestingMode(const PacketMetadata &metadata)
    {
        FileNestingMode mode = FileNestingMode::Unknown;
        if(metadata.cannot_use_wad && metadata.key_only)
        {
            mode = FileNestingMode::SimpleGPGWithKey;
        }
        else if(metadata.key_only)
        {
            mode = FileNestingMode::WadWithGPG;
        }
        else if(!metadata.key_file.empty())
        {
            mode = FileNestingMode::NestedGPGWithWad;
        }
        else
        {
            mode = FileNestingMode::SimpleGPG;
        }
        return mode;
    }

    template<typename T>
    EpadResult UpdateOrFinish(T &reader_writer, SafeVector &buffer,  bool finish)
    {
        try
        {
            if(finish)
            {
                reader_writer.Finish(buffer);
            }
            else
            {
                reader_writer.Update(buffer);
            }
        }
        catch(const EmsgException &e)
        {
            return ToEpadResult(e.result);
        }
        return EpadResult::Success;
    }

    EpadResult EncryptStream(InStream &in, EncryptParams &encrypt_params,
            OutStream &out, PacketMetadata &metadata)
    {
        MessageConfig passphrase_config;
        MessageConfig key_file_config;
        SymmetricEncryptionSession passphrase_session;
        SymmetricEncryptionSession key_file_session;

        auto mode = IdentifyFileNestingMode(metadata);
        bool is_passphrase_session = (mode == FileNestingMode::SimpleGPG || mode == FileNestingMode::NestedGPGWithWad);
        bool is_key_file_session = (mode == FileNestingMode::SimpleGPGWithKey || mode == FileNestingMode::WadWithGPG ||
                mode == FileNestingMode::NestedGPGWithWad);

        MessageWriter passphrase_session_writer;
        MessageWriter key_file_session_writer;
        if(is_passphrase_session)
        {
            passphrase_config = ConvertToMessageConfig(metadata);
            passphrase_session = PreparePassphraseSession(encrypt_params, passphrase_config);
            passphrase_session_writer.Start(std::move(passphrase_session.key), passphrase_config, passphrase_session.salt);
        }

        if(is_key_file_session)
        {
            key_file_config = ConvertToMessageConfig(metadata);
            key_file_session = PrepareKeyFileSession(encrypt_params, metadata, key_file_config);
            key_file_session_writer.Start(std::move(key_file_session.key), key_file_config, key_file_session.salt);
        }

        bool wad_head_written = false;
        std::string wad_key_file = metadata.persist_key_path ? metadata.key_file : std::string();
        //The payload is always last in the file. So its size is not necessary to store
        //Sadly, we have to violate WAD specification for files that are bigger than the buffer
        //We will only use this variable if the file is bigger than the buffer size
        uint32_t wad_payload_size = 0;
        std::string wad_version = "1";

        // Progress counters
        ProgressEvent progress_event;
        progress_event.total_bytes = in.GetCount();
        progress_event.complete_bytes = 0;

        SafeVector buf;
        // Use do while to process empty files
        do
        {
            EpadResult result = EpadResult::None;
            buf.resize(std::min(in.GetCount(), static_cast<stream_length_type>(encrypt_params.memory_buffer)));
            stream_length_type length = in.Read(buf.data(), buf.size());
            buf.resize(length);
            MessageWriter *writer = nullptr;
            progress_event.complete_bytes += length;
            encrypt_params.progress_callback(progress_event);
            if(progress_event.cancel)
            {
                result = EpadResult::Cancelled;
                return result;
            }

            switch(mode)
            {
                case FileNestingMode::SimpleGPG:
                case FileNestingMode::SimpleGPGWithKey:

                    writer = (mode == FileNestingMode::SimpleGPGWithKey ?
                       &key_file_session_writer : &passphrase_session_writer);

                    result = UpdateOrFinish(*writer, buf, in.IsEOF());
                    if(result != EpadResult::Success)
                        return result;

                    break;

                case FileNestingMode::WadWithGPG:
                    result = UpdateOrFinish(key_file_session_writer, buf, in.IsEOF());
                    if(result != EpadResult::Success)
                        return result;

                    if(!wad_head_written)
                    {
                        if(in.IsEOF())
                        {
                            //We know the payload size
                            wad_payload_size = buf.size();
                            wad_version = "0";
                        }

                        if(!WriteWadHead(wad_key_file, wad_payload_size, wad_version, out))
                            return EpadResult::IOErrorOutput;
                        wad_head_written = true;
                    }
                    break;

                case FileNestingMode::NestedGPGWithWad:
                    result = UpdateOrFinish(key_file_session_writer, buf, in.IsEOF());
                    if(result != EpadResult::Success)
                        return result;

                    if(!wad_head_written)
                    {
                        if(in.IsEOF())
                        {
                            //We know the payload size
                            wad_payload_size = buf.size();
                            wad_version = "0";
                        }
                        SafeVector wad_head;
                        auto wad_head_out = MakeOutStream(wad_head);
                        if(!WriteWadHead(wad_key_file, wad_payload_size, wad_version, *wad_head_out))
                            return EpadResult::IOErrorOutput;
                        wad_head.resize(wad_head_out->GetCount());
                        buf.insert(buf.begin(), wad_head.cbegin(), wad_head.cend());
                        wad_head_written = true;
                    }

                    result = UpdateOrFinish(passphrase_session_writer, buf, in.IsEOF());
                    if(result != EpadResult::Success)
                        return result;

                    break;

                default:
                    assert(false); // Unknown mode
                    break;
            }

            // All cases above should leave buf for writing to the out stream
            if(!out.Write(buf.data(), buf.size()))
                return EpadResult::IOErrorOutput;
        }
        while(!in.IsEOF());

        return EpadResult::Success;
    }

    // Factory method that opens file_in. If file_in is a pipe or it is not seekable for another reason,
    // fall_back_buffer is used to read the whole input and the returned stream reads from the
    // buffer
    // Returns empty unique_ptr if fails
    std::unique_ptr<RandomInStream> CreateInStream(const std::string &file_in, std::vector<byte> &fall_back_buffer)
    {
        InPacketStreamFile *in_stm_file;
        std::unique_ptr<RandomInStream> in_stm(in_stm_file = new InPacketStreamFile());

        OpenFileResult result = OpenFile(file_in, *in_stm_file);

        if(result == OpenFileResult::NotSeekable && file_in == "-")
        {
            in_stm.reset();
            if(!LoadFromIOStream(GetStdinNo(), fall_back_buffer))
                return in_stm;

            in_stm = std::unique_ptr<RandomInStream>(new InPacketStreamMemory(fall_back_buffer.data(), fall_back_buffer.data()
                        + fall_back_buffer.size()));
        }
        else if(result != OpenFileResult::OK)
        {
            in_stm.reset();
        }

        return in_stm;
    }
}

namespace EncryptPad
{
    EpadResult EncryptBuffer(const Botan::SecureVector<byte> &input_buffer, EncryptParams &encrypt_params,
             Botan::SecureVector<byte> &output_buffer, PacketMetadata &metadata)
    {
        InPacketStreamMemory in(input_buffer.data(), input_buffer.data() + input_buffer.size());
        output_buffer.clear();
        auto out = MakeOutStream(output_buffer);
        auto result = EncryptStream(in, encrypt_params, *out, metadata);
        if(result != EpadResult::Success)
            return result;
        output_buffer.resize(out->GetCount());
        return result;
    }

    EpadResult DecryptBuffer(const Botan::SecureVector<byte> &input_buffer, const EncryptParams &encrypt_params,
             Botan::SecureVector<byte> &output_buffer, PacketMetadata &metadata)
    {
        InPacketStreamMemory in(input_buffer.data(), input_buffer.data() + input_buffer.size());
        output_buffer.clear();
        auto out = MakeOutStream(output_buffer);
        auto result = DecryptStream(in, encrypt_params, *out, metadata);
        if(result != EpadResult::Success)
            return result;
        output_buffer.resize(out->GetCount());
        return result;
    }

    EpadResult EncryptPacketFile(const Buffer &input_buffer, const std::string &file_out, 
            EncryptParams &encrypt_params, PacketMetadata &metadata)
    {
        EpadResult result = EpadResult::None;

        {
            InPacketStreamMemory stm_in(input_buffer.data(), input_buffer.data() + input_buffer.size());

            OutPacketStreamFile out;
            if(OpenFile(file_out, out) != OpenFileResult::OK)
                return EpadResult::IOErrorOutput;

            result = EncryptStream(stm_in, encrypt_params, out, metadata); 
        }

        if(result != EpadResult::Success)
        {
            RemoveFile(file_out);
        }
        return result;
    }

    EpadResult EncryptPacketFile(const std::string &file_in, const std::string &file_out,
            EncryptParams &encrypt_params, PacketMetadata &metadata)
    {
        std::vector<byte> fall_back_buffer;
        std::unique_ptr<RandomInStream> in = CreateInStream(file_in, fall_back_buffer);
        if(in.get() == nullptr)
            return EpadResult::IOErrorInput;

        OutPacketStreamFile out;
        if(OpenFile(file_out, out) != OpenFileResult::OK)
        {
            return EpadResult::IOErrorOutput;
        }

        EpadResult epad_result = EncryptStream(*in, encrypt_params, out, metadata);

        if(epad_result != EpadResult::Success && file_out != "-")
        {
            RemoveFile(file_out);
        }
        return epad_result;
    }

    EpadResult DecryptPacketFile(const std::string &file_in, const EncryptParams &encrypt_params, 
            Buffer &output_buffer, PacketMetadata &metadata)
    {
        InPacketStreamFile in;
        OpenFileResult result = OpenFile(file_in, in);
        if(result != OpenFileResult::OK)
            return EpadResult::IOErrorInput;

        auto out_stm = MakeOutStream(output_buffer);
        EpadResult epad_result = DecryptStream(in, encrypt_params, *out_stm, metadata);
        output_buffer.resize(out_stm->GetCount());
        return epad_result;
    }

    EpadResult DecryptPacketFile(const std::string &file_in, const std::string &file_out,
            const EncryptParams &encrypt_params, PacketMetadata &metadata)
    {
        InPacketStreamFile in;
        OpenFileResult result = OpenFile(file_in, in);
        if(result != OpenFileResult::OK)
            return EpadResult::IOErrorInput;

        OutPacketStreamFile out;
        if(OpenFile(file_out, out) != OpenFileResult::OK)
        {
            return EpadResult::IOErrorOutput;
        }

        return DecryptStream(in, encrypt_params, out, metadata);
    }

    bool CheckIfPassphraseProtected(const std::string &file_name, bool &wad_file, std::string &key_file)
    {
        wad_file = false;
        key_file.clear();
        InPacketStreamFile stm;
        if(OpenFile(file_name, stm) != OpenFileResult::OK)
            return false;

        int b = stm.Get();
        if(b == -1)
            return false;

        // gpg should have this bit set
        // 0xEF is BOM or a new format 47 packet that is not known to us. Let's assume it is BOM.
        if(b & 0x80 && b != 0xEF)
        {
            return true;
        }

        stm.Seek(0);

        auto result = ExtractKeyFromWad(stm, key_file);

        if(result != EpadResult::Success)
            return false;

        wad_file = true;
        return false;
    }

    bool CheckIfKeyFileMayRequirePassphrase(const std::string &key_file)
    {
        std::string empty;
        std::string key_content;
        if(IsUrl(key_file))
            return true;

        auto result = LoadKeyFromFile(key_file, empty, empty, key_content);
        if(result != EpadResult::Success || IsKeyFileEncrypted(key_content))
            return true;

        return false;
    }
}
