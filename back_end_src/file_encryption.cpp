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

    PacketResult ReadPacket(InStream &in, OutStream &out,
            const EncryptParams &enc_params, PacketMetadata &metadata)
    {
        KeyService *key_service = enc_params.key_service;
        KeyServiceKeyProvider key_provider(key_service, enc_params.passphrase);
        MessageReader reader;
        reader.Start(key_provider);
        SafeVector buf;
        buf.resize(in.GetCount());
        stream_length_type length = in.Read(buf.data(), in.GetCount());
        assert(static_cast<size_t>(length) == buf.size());
        buf.resize(length);
        try
        {
            reader.Finish(buf);
        }
        catch(const EmsgException &e)
        {
            return e.result;
        }
        if(!out.Write(buf.data(), buf.size()))
        {
            return PacketResult::IOErrorOutput;
        }

        return PacketResult::Success;
    }

    PacketResult WritePacket(InStream &in, OutStream &out,
            EncryptParams &enc_params, PacketMetadata &metadata)
    {
        MessageConfig config = ConvertToMessageConfig(metadata);
        std::unique_ptr<EncryptionKey> encryption_key;
        Salt salt;
        if(enc_params.passphrase)
        {
            Passphrase passphrase(SafeVector(enc_params.passphrase->begin(), enc_params.passphrase->end()));
            salt = GenerateRandomSalt();
            encryption_key = GenerateEncryptionKey(passphrase, config.GetCipherAlgo(),
                    config.GetHashAlgo(), config.GetIterations(), salt);
        }
        else
        {
            const KeyRecord &key_record = enc_params.key_service->GetKeyForSaving();
            salt = key_record.salt;
            // We assume that key_service has been set with the same encryption parameters
            encryption_key.reset(new EncryptionKey(key_record.key->bits_of()));
        }

        SafeVector buf;
        buf.resize(in.GetCount());
        stream_length_type length = in.Read(buf.data(), in.GetCount());
        assert(static_cast<size_t>(length) == buf.size());
        buf.resize(length);

        MessageWriter writer;
        writer.Start(std::move(encryption_key), config, salt);
        try
        {
            writer.Finish(buf);
        }
        catch(const EmsgException &e)
        {
            return e.result;
        }

        if(!out.Write(buf.data(), buf.size()))
        {
            return PacketResult::IOErrorOutput;
        }

        return PacketResult::Success;
    }

    PacketResult EncryptWithKey(InStream &in, EncryptParams &encrypt_params, 
        OutStream &out, PacketMetadata &metadata)
    {
        //TODO: remove this code because it moved to PrepareKeyFileSession
        std::string raw_key_phrase, key_phrase;
        std::string empty_str;
        auto result = LoadKeyFromFile(metadata.key_file, 
                encrypt_params.libcurl_path ? *encrypt_params.libcurl_path : empty_str,
                encrypt_params.libcurl_parameters ? *encrypt_params.libcurl_parameters : empty_str,
                raw_key_phrase);

        if(result != PacketResult::Success)
            return result;

        if(!DecryptKeyFileContent(raw_key_phrase, encrypt_params.key_file_encrypt_params, key_phrase)) 
            return PacketResult::InvalidKeyFilePassphrase;

        KeyService key_service(1);
        key_service.ChangePassphrase(key_phrase, metadata.hash_algo, 
                GetAlgoSpec(metadata.cipher_algo).key_size, metadata.iterations);
        EncryptParams enc_params = {};
        enc_params.key_service = &key_service;

        return WritePacket(in, out, enc_params, metadata);
    }

    PacketResult EncryptWad(InStream &in, EncryptParams &encrypt_params, OutStream &out, PacketMetadata &metadata)
    {
        Buffer buffer;
        auto buffer_out = MakeOutStream(buffer);
        auto result = EncryptWithKey(in, encrypt_params, *buffer_out, metadata);
        if(result != PacketResult::Success)
            return result;

        InPacketStreamMemory buffer_in(buffer.data(), buffer.data() + buffer_out->GetCount());
        WriteWad(buffer_in, out, 
                metadata.persist_key_path ? metadata.key_file : std::string());
        return PacketResult::Success;
    }

    struct SymmetricEncryptionSession
    {
        std::unique_ptr<EncryptionKey> key;
        Salt salt;
        PacketResult preparation_result;
        SymmetricEncryptionSession():preparation_result(PacketResult::None)
        {
        }

        bool IsValid()
        {
            return preparation_result == PacketResult::Success;
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
            ret_val.preparation_result = PacketResult::Success;
        }
        else
        {
            const KeyRecord &key_record = encrypt_params.key_service->GetKeyForSaving();
            ret_val.salt = key_record.salt;
            // We assume that key_service has been set with the same encryption parameters
            ret_val.key.reset(new EncryptionKey(key_record.key->bits_of()));
            ret_val.preparation_result = PacketResult::Success;
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

        if(session.preparation_result != PacketResult::Success)
            return session;

        //TODO: key_phrase should be SafeVector
        if(!DecryptKeyFileContent(raw_key_phrase, encrypt_params.key_file_encrypt_params, key_phrase))
        {
            session.preparation_result = PacketResult::InvalidKeyFilePassphrase;
            return session;
        }
        Passphrase passphrase(SafeVector(key_phrase.cbegin(), key_phrase.cend()));
        std::fill(key_phrase.begin(), key_phrase.end(), '0');
        session.salt = GenerateRandomSalt();
        session.key = GenerateEncryptionKey(passphrase, config.GetCipherAlgo(),
                config.GetHashAlgo(), config.GetIterations(), session.salt);
        session.preparation_result = PacketResult::Success;
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
    PacketResult UpdateOrFinish(T &reader_writer, SafeVector &buffer,  bool finish)
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
            return e.result;
        }
        return PacketResult::Success;
    }

    PacketResult EncryptStream(InStream &in, EncryptParams &encrypt_params,
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

        SafeVector buf;
        // Use do while to process empty files
        do
        {
            buf.resize(std::min(in.GetCount(), static_cast<stream_length_type>(encrypt_params.memory_buffer)));
            stream_length_type length = in.Read(buf.data(), buf.size());
            buf.resize(length);
            MessageWriter *writer = nullptr;
            PacketResult result = PacketResult::None;


            switch(mode)
            {
                case FileNestingMode::SimpleGPG:
                case FileNestingMode::SimpleGPGWithKey:

                    writer = (mode == FileNestingMode::SimpleGPGWithKey ?
                       &key_file_session_writer : &passphrase_session_writer);

                    result = UpdateOrFinish(*writer, buf, in.IsEOF());
                    if(result != PacketResult::Success)
                        return result;

                    break;

                case FileNestingMode::WadWithGPG:
                    result = UpdateOrFinish(key_file_session_writer, buf, in.IsEOF());
                    if(result != PacketResult::Success)
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
                            return PacketResult::IOErrorOutput;
                        wad_head_written = true;
                    }
                    break;

                case FileNestingMode::NestedGPGWithWad:
                    result = UpdateOrFinish(key_file_session_writer, buf, in.IsEOF());
                    if(result != PacketResult::Success)
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
                            return PacketResult::IOErrorOutput;
                        wad_head.resize(wad_head_out->GetCount());
                        buf.insert(buf.begin(), wad_head.cbegin(), wad_head.cend());
                        wad_head_written = true;
                    }

                    result = UpdateOrFinish(passphrase_session_writer, buf, in.IsEOF());
                    if(result != PacketResult::Success)
                        return result;

                    break;

                default:
                    assert(false); // Unknown mode
                    break;
            }

            // All cases above should leave buf for writing to the out stream
            if(!out.Write(buf.data(), buf.size()))
                return PacketResult::IOErrorOutput;
        }
        while(!in.IsEOF());

        return PacketResult::Success;
    }

    PacketResult DecryptWad(RandomInStream &in, const std::string &key_file, 
            const EncryptParams &encrypt_params, OutStream &out, PacketMetadata &metadata);

    PacketResult DecryptStream(InStream &in_stm, const EncryptParams &enc_params, 
            OutStream &out_stm, PacketMetadata &metadata)
    {
        Buffer buffer;
        auto buffer_out = MakeOutStream(buffer);
        PacketResult result = ReadPacket(in_stm, *buffer_out, enc_params, metadata);
        if(result != PacketResult::Success)
            return result;

        std::string marker;

        if(buffer.size() >= 4)
        {
            marker.insert(0U, reinterpret_cast<const char*>(buffer.data()), 4U);
        }

        if(marker == "IWAD" || marker == "PWAD")
        {
            InPacketStreamMemory buffer_in(buffer.data(), buffer.data() + buffer_out->GetCount());
            result = DecryptWad(buffer_in, metadata.key_file, enc_params, out_stm, metadata);
            if(result == PacketResult::InvalidSurrogateIV)
                return PacketResult::InvalidKeyFile;

            if(result != PacketResult::Success)
                return result;
        }
        else
        {
            out_stm.Write(buffer.data(), buffer_out->GetCount());
        }
        
        return PacketResult::Success;
    }

    PacketResult DecryptWithKey(InStream &in, const EncryptParams &encrypt_params,
            OutStream &out, PacketMetadata &metadata)
    {
        if(metadata.key_file.empty())
            return PacketResult::KeyFileNotSpecified;

        std::string passphrase_from_key;
        EncryptParams enc_params = {};
        enc_params.passphrase = &passphrase_from_key;
        KeyService key_service(1);
        enc_params.key_service = &key_service;

        std::string empty_str;
        auto result = LoadKeyFromFile(metadata.key_file, 
                encrypt_params.libcurl_path ? *encrypt_params.libcurl_path : empty_str,
                encrypt_params.libcurl_parameters ? *encrypt_params.libcurl_parameters : empty_str,
                passphrase_from_key);

        if(result != PacketResult::Success)
            return result;

        if(!DecryptKeyFileContent(passphrase_from_key, encrypt_params.key_file_encrypt_params, passphrase_from_key))
            return PacketResult::InvalidKeyFilePassphrase;

        return DecryptStream(in, enc_params, out, metadata);
    }

    // Extracts data from wad file and decrypts it.
    PacketResult DecryptWad(RandomInStream &in, const std::string &key_file, 
            const EncryptParams &encrypt_params, OutStream &out, PacketMetadata &metadata)
    {
        Buffer payload;
        auto payload_out = MakeOutStream(payload);
        metadata.key_file = key_file;
        std::string key_file_tmp;
        auto result = ExtractFromWad(in, *payload_out, key_file_tmp);
        if(result != PacketResult::Success)
            return result;

        if(!key_file_tmp.empty())
            metadata.persist_key_path = true;

        if(metadata.key_file.empty())
        {
            metadata.key_file = key_file_tmp;
        }

        InPacketStreamMemory payload_stm(payload.data(), payload.data() + payload_out->GetCount());
        return DecryptWithKey(payload_stm, encrypt_params, out, metadata);
    }

    PacketResult WriteFile(const std::string &file_name, const Buffer &stm)
    {
        OutPacketStreamFile out;
        if(OpenFile(file_name, out) != OpenFileResult::OK)
            return PacketResult::IOErrorOutput;

        if(!out.Write(stm.data(), stm.size()))
            return PacketResult::IOErrorOutput;

        return PacketResult::Success;
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

    std::pair<PacketResult, FileNestingMode> IdentifyFileNestingMode(RandomInStream &in, const EncryptParams &encrypt_params,
            const PacketMetadata &metadata)
    {
        const int kInvalid = -1;

        std::pair<PacketResult, FileNestingMode> pair;
        pair.first = PacketResult::None;
        pair.second = FileNestingMode::Unknown;

        int b = in.Get();

        // Check if the file is empty
        if(b == kInvalid)
        {
            pair.first = PacketResult::UnexpectedFormat;
            return pair;
        }

        in.Seek(0);
        // gpg should have this bit set
        // 0xEF is BOM or a new format 47 packet that is not known to us. Let's assume it is BOM.
        if(b & 0x80 && b != 0xEF)
        {
            pair.first = PacketResult::Success;
            pair.second = (metadata.key_only) ?
                // It is a file with GPG extension that doesn't support WAD format
                FileNestingMode::SimpleGPGWithKey
                :
                FileNestingMode::SimpleGPGOrNestedGPGWithWad;
            return pair;
        }
        else // wad starts from I or P, in which the most significant bit is not set
        {
            pair.first = PacketResult::Success;
            pair.second = FileNestingMode::WadWithGPG;
            return pair;
        }
    }

    struct DecryptionSession
    {
        MessageReader reader;
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

    using SessionResult = std::pair<PacketResult, std::unique_ptr<DecryptionSession> >;
    SessionResult CreateKeyFileDecryptionSession(const EncryptParams &encrypt_params,
            PacketMetadata &metadata)
    {
        SessionResult result;
        if(metadata.key_file.empty())
        {
            result.first = PacketResult::KeyFileNotSpecified;
            return result;
        }

        result.second.reset(new DecryptionSession());
        std::string empty_str;
        result.first = LoadKeyFromFile(metadata.key_file, 
                encrypt_params.libcurl_path ? *encrypt_params.libcurl_path : empty_str,
                encrypt_params.libcurl_parameters ? *encrypt_params.libcurl_parameters : empty_str,
                result.second->own_passphrase);

        if(result.first != PacketResult::Success)
            return result;

        if(!DecryptKeyFileContent(result.second->own_passphrase,
                    encrypt_params.key_file_encrypt_params, result.second->own_passphrase))
        {
            result.first = PacketResult::InvalidKeyFilePassphrase;
            return result;
        }

        result.first = PacketResult::Success;
        return result;
    }

    PacketResult DecryptPacketFileToStream(const std::string &file_in, const EncryptParams &encrypt_params,
            OutStream &out, PacketMetadata &metadata)
    {
        std::vector<byte> fall_back_buffer;
        std::unique_ptr<RandomInStream> in = CreateInStream(file_in, fall_back_buffer);
        if(in.get() == nullptr)
            return PacketResult::IOErrorInput;

        auto mode_pair = IdentifyFileNestingMode(*in, encrypt_params, metadata);
        if(mode_pair.first != PacketResult::Success)
            return mode_pair.first;

        FileNestingMode mode = mode_pair.second;
        if(mode == FileNestingMode::WadWithGPG)
            metadata.key_only = true;

        std::unique_ptr<DecryptionSession> passphrase_session;
        std::unique_ptr<DecryptionSession> key_file_session;

        switch(mode)
        {
            case FileNestingMode::SimpleGPG:
            case FileNestingMode::NestedGPGWithWad:
            case FileNestingMode::SimpleGPGOrNestedGPGWithWad:
                passphrase_session.reset(new DecryptionSession(encrypt_params.key_service, encrypt_params.passphrase));
                break;
            default:
                //we don't know yet
                break;
        }

        if(!metadata.key_file.empty() && mode != FileNestingMode::SimpleGPG)
        {
            // The preset key file takes precedence over the persistent key file
            auto key_file_session_result = CreateKeyFileDecryptionSession(encrypt_params, metadata);
            if(key_file_session_result.first != PacketResult::Success)
                return key_file_session_result.first;

            key_file_session = std::move(key_file_session_result.second);
        }
        //TODO: This buffer size if for only testing small buffers
        const size_t kBufferSize = 16;
        SafeVector buffer;
        SafeVector accumulated_pre_buffer;
        SafeVector accumulated_post_buffer;

        while(!in->IsEOF())
        {
            buffer.resize(std::min(static_cast<size_t>(in->GetCount()), kBufferSize));
            auto length = in->Read(buffer.data(), buffer.size());
            buffer.resize(length);

            PacketResult result = PacketResult::None;
            switch(mode)
            {
                case FileNestingMode::SimpleGPG:
                case FileNestingMode::SimpleGPGWithKey:
                    {
                        MessageReader *reader = nullptr;
                        reader = (mode == FileNestingMode::SimpleGPG) ?
                            &passphrase_session->reader :
                            &key_file_session->reader;

                        if(accumulated_pre_buffer.size() > 0)
                        {
                            buffer.insert(buffer.begin(), accumulated_pre_buffer.begin(), accumulated_pre_buffer.end());
                            accumulated_pre_buffer.clear();
                        }

                        result = UpdateOrFinish(*reader, buffer, in->IsEOF());
                        if(result != PacketResult::Success)
                            return result;

                        if(accumulated_post_buffer.size() > 0)
                        {
                            buffer.insert(buffer.begin(), accumulated_post_buffer.begin(), accumulated_post_buffer.end());
                            accumulated_post_buffer.clear();
                        }
                    }
                    break;
                case FileNestingMode::WadWithGPG:
                    {
                        accumulated_pre_buffer.insert(accumulated_pre_buffer.end(), buffer.begin(), buffer.end());
                        buffer.clear();
                        InPacketStreamMemory accumulated_in(accumulated_pre_buffer.data(),
                                accumulated_pre_buffer.data() + accumulated_pre_buffer.size());

                        uint32_t payload_offset = 0;
                        result = ParseWad(accumulated_in, metadata.key_file, payload_offset);

                        if(result != PacketResult::Success && result != PacketResult::InvalidOrIncompleteWadFile)
                            return result;

                        if(result != PacketResult::Success)
                            break;

                        //erase the wad head
                        assert(accumulated_pre_buffer.size() >= payload_offset);
                        accumulated_pre_buffer.erase(accumulated_pre_buffer.begin(),
                                accumulated_pre_buffer.begin() + payload_offset);

                        if(!key_file_session)
                        {
                            // The key wasn't preset and we recovered the key file location from the wad file
                            auto key_file_session_result = CreateKeyFileDecryptionSession(encrypt_params, metadata);
                            if(key_file_session_result.first != PacketResult::Success)
                                return key_file_session_result.first;

                            key_file_session = std::move(key_file_session_result.second);
                        }

                        // The rest of the file is simple gpg with key
                        mode = FileNestingMode::SimpleGPGWithKey;
                    }
                    break;
                case FileNestingMode::SimpleGPGOrNestedGPGWithWad:
                    {
                        result = UpdateOrFinish(passphrase_session->reader, buffer, in->IsEOF());
                        if(result != PacketResult::Success)
                            return result;

                        accumulated_post_buffer.insert(accumulated_post_buffer.end(), buffer.begin(), buffer.end());
                        buffer.clear();

                        if(accumulated_post_buffer.size() >= 4)
                        {
                            std::string marker;
                            marker.insert(0U, reinterpret_cast<const char*>(accumulated_post_buffer.data()), 4U);
                            if(marker == "IWAD" || marker == "PWAD")
                            {
                                mode = FileNestingMode::NestedGPGWithWad;
                            }
                            else
                            {
                                mode = FileNestingMode::SimpleGPG;
                            }
                        }
                    }
                    break;
                case FileNestingMode::NestedGPGWithWad:
                    {
                        assert(false);//Not implemented
                    }
                    break;
                default:
                    assert(false);
                    break;
            }

            out.Write(buffer.data(), buffer.size());
        }
        return PacketResult::Success;
    }

#if 0
    PacketResult DecryptPacketFileToStream(const std::string &file_in, const EncryptParams &encrypt_params,
            OutStream &out_stm, PacketMetadata &metadata)
    {
        const int kInvalid = -1;

        std::vector<byte> fall_back_buffer;
        std::unique_ptr<RandomInStream> stm = CreateInStream(file_in, fall_back_buffer);
        if(stm.get() == nullptr)
            return PacketResult::IOErrorInput;

        int b = stm->Get();

        // Check if the file is empty
        if(b == kInvalid)
            return PacketResult::UnexpectedFormat;

        stm->Seek(0);
        // gpg should have this bit set
        // 0xEF is BOM or a new format 47 packet that is not known to us. Let's assume it is BOM.
        if(b & 0x80 && b != 0xEF)
        {
            PacketResult result = PacketResult::None;
            if(metadata.key_only)
            {
                // It is a file with GPG extension that doesn't support WAD format
                result = DecryptWithKey(*stm, encrypt_params, out_stm, metadata);
                if(result == PacketResult::InvalidSurrogateIV)
                    result = PacketResult::InvalidKeyFile;
            }
            else
            {
                result = DecryptStream(*stm, encrypt_params, out_stm, metadata);
            }
            return result;
        }
        else // wad starts from I or P, in which the most significant bit is not set
        {
            auto result = DecryptWad(*stm, metadata.key_file, encrypt_params, out_stm, metadata);

            if(result == PacketResult::InvalidWadFile)
                return PacketResult::UnexpectedFormat;
            if(result == PacketResult::InvalidSurrogateIV)
                return PacketResult::InvalidKeyFile;

            metadata.key_only = true;
            return result;
        }
    }
#endif

}

namespace EncryptPad
{
    PacketResult EncryptBuffer(const Botan::SecureVector<byte> &input_buffer, EncryptParams &encrypt_params,
             Botan::SecureVector<byte> &output_buffer, PacketMetadata &metadata)
    {
        InPacketStreamMemory in(input_buffer.data(), input_buffer.data() + input_buffer.size());
        output_buffer.clear();
        auto out = MakeOutStream(output_buffer);
        return EncryptStream(in, encrypt_params, *out, metadata);
    }

    PacketResult DecryptBuffer(const Botan::SecureVector<byte> &input_buffer, const EncryptParams &encrypt_params,
             Botan::SecureVector<byte> &output_buffer, PacketMetadata &metadata)
    {
        InPacketStreamMemory in(input_buffer.data(), input_buffer.data() + input_buffer.size());
        output_buffer.clear();
        auto out = MakeOutStream(output_buffer);
        return DecryptStream(in, encrypt_params, *out, metadata);
    }

    PacketResult EncryptPacketFile(const Buffer &input_buffer, const std::string &file_out, 
            EncryptParams &encrypt_params, PacketMetadata &metadata)
    {
        PacketResult result = PacketResult::None;

        {
            InPacketStreamMemory stm_in(input_buffer.data(), input_buffer.data() + input_buffer.size());

            OutPacketStreamFile out;
            if(OpenFile(file_out, out) != OpenFileResult::OK)
                return PacketResult::IOErrorOutput;

            result = EncryptStream(stm_in, encrypt_params, out, metadata); 
        }

        if(result != PacketResult::Success)
        {
            RemoveFile(file_out);
        }
        return result;
    }

    PacketResult EncryptPacketFile(const std::string &file_in, const std::string &file_out, 
            EncryptParams &encrypt_params, PacketMetadata &metadata)
    {
        std::vector<byte> fall_back_buffer;
        std::unique_ptr<RandomInStream> in = CreateInStream(file_in, fall_back_buffer);
        if(in.get() == nullptr)
            return PacketResult::IOErrorInput;

        OutPacketStreamFile out;
        if(OpenFile(file_out, out) != OpenFileResult::OK)
        {
            return PacketResult::IOErrorOutput;
        }

        PacketResult result = EncryptStream(*in, encrypt_params, out, metadata);

        if(result != PacketResult::Success && file_out != "-")
        {
            RemoveFile(file_out);
        }
        return result;
    }

    PacketResult DecryptPacketFile(const std::string &file_in, const EncryptParams &encrypt_params, 
            Buffer &output_buffer, PacketMetadata &metadata)
    {
        auto out_stm = MakeOutStream(output_buffer);
        PacketResult result = DecryptPacketFile(file_in, encrypt_params, output_buffer, metadata);
        output_buffer.resize(out_stm->GetCount());
        return result;
    }

    PacketResult DecryptPacketFile(const std::string &file_in, const std::string &file_out,
            const EncryptParams &encrypt_params, PacketMetadata &metadata)
    {
        OutPacketStreamFile out;
        if(OpenFile(file_out, out) != OpenFileResult::OK)
        {
            return PacketResult::IOErrorOutput;
        }

        auto result = DecryptPacketFileToStream(file_in, encrypt_params, out, metadata);
        if(result != PacketResult::Success)
            return result;

        return result;
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

        if(result != PacketResult::Success)
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
        if(result != PacketResult::Success || IsKeyFileEncrypted(key_content))
            return true;

        return false;
    }
}
