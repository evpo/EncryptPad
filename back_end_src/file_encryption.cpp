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
#include "packet_composer.h"
#include "wad_reader_writer.h"
#include "x2_key_loader.h"
#include "epad_utilities.h"
#include "file_helper.h"
#include "win_file_reader.h"
#include "key_file_converter.h"

namespace 
{
    using namespace EncryptPad;
    typedef Botan::SecureVector<byte> Buffer;

    PacketResult EncryptWithKey(InStream &in, EncryptParams &encrypt_params, 
        OutStream &out, PacketMetadata &metadata)
    {
        std::string raw_key_phrase, key_phrase;
        std::string empty_str;
        auto result = LoadKeyFromFile(metadata.key_file, 
                encrypt_params.libcurl_path ? *encrypt_params.libcurl_path : empty_str, 
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

        InPacketStreamMemory buffer_in(buffer.begin(), buffer.begin() + buffer_out->GetCount());
        WriteWad(buffer_in, out, 
                metadata.persist_key_path ? metadata.key_file : std::string());
        return PacketResult::Success;
    }

    PacketResult EncryptStream(InStream &in, EncryptParams &encrypt_params, 
            OutStream &out, PacketMetadata &metadata)
    {
        if(metadata.cannot_use_wad && metadata.key_only)
        {
            return EncryptWithKey(in, encrypt_params, out, metadata);
        }
        if(metadata.key_only)
        {
            return EncryptWad(in, encrypt_params, out, metadata);
        }
        else if(!metadata.key_file.empty())
        {
            Buffer buffer;
            auto buffer_out = MakeOutStream(buffer);
            auto result = EncryptWad(in, encrypt_params, *buffer_out, metadata);
            if(result != PacketResult::Success)
                return result;

            InPacketStreamMemory buffer_in(buffer.begin(), buffer.begin() + buffer_out->GetCount());

            return WritePacket(buffer_in, out, encrypt_params, metadata);
        }
        else
        {
            return WritePacket(in, out, encrypt_params, metadata);
        }
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

        if(buffer_out->GetCount() < 4)
            return PacketResult::Success;

        std::string marker(buffer.begin(), buffer.begin() + 4);

        if(marker == "IWAD" || marker == "PWAD")
        {
            InPacketStreamMemory buffer_in(buffer.begin(), buffer.begin() + buffer_out->GetCount());
            result = DecryptWad(buffer_in, metadata.key_file, enc_params, out_stm, metadata);
            if(result == PacketResult::InvalidSurrogateIV)
                return PacketResult::InvalidKeyFile;

            if(result != PacketResult::Success)
                return result;
        }
        else
        {
            out_stm.Write(buffer.begin(), buffer_out->GetCount());
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

        InPacketStreamMemory payload_stm(payload.begin(), payload.begin() + payload_out->GetCount());
        return DecryptWithKey(payload_stm, encrypt_params, out, metadata);
    }

    PacketResult WriteFile(const std::string &file_name, const Buffer &stm)
    {
        OutPacketStreamFile out;
        if(OpenFile(file_name, out) != OpenFileResult::OK)
            return PacketResult::IOErrorOutput;

        if(!out.Write(stm.begin(), stm.size()))
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
}

namespace EncryptPad
{
    PacketResult EncryptBuffer(const Botan::SecureVector<byte> &input_buffer, EncryptParams &encrypt_params,
             Botan::SecureVector<byte> &output_buffer, PacketMetadata &metadata)
    {
        InPacketStreamMemory in(input_buffer.begin(), input_buffer.end());
        output_buffer.clear();
        auto out = MakeOutStream(output_buffer);
        return EncryptStream(in, encrypt_params, *out, metadata);
    }

    PacketResult DecryptBuffer(const Botan::SecureVector<byte> &input_buffer, const EncryptParams &encrypt_params,
             Botan::SecureVector<byte> &output_buffer, PacketMetadata &metadata)
    {
        InPacketStreamMemory in(input_buffer.begin(), input_buffer.end());
        output_buffer.clear();
        auto out = MakeOutStream(output_buffer);
        return DecryptStream(in, encrypt_params, *out, metadata);
    }

    PacketResult EncryptPacketFile(const Buffer &input_buffer, const std::string &file_out, 
            EncryptParams &encrypt_params, PacketMetadata &metadata)
    {
        PacketResult result = PacketResult::None;

        {
            InPacketStreamMemory stm_in(input_buffer.begin(), input_buffer.end());

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
        const int kInvalid = -1;

        std::vector<byte> fall_back_buffer;
        std::unique_ptr<RandomInStream> stm = CreateInStream(file_in, fall_back_buffer);
        if(stm.get() == nullptr)
            return PacketResult::IOErrorInput;

        auto out_stm = MakeOutStream(output_buffer);

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
                result = DecryptWithKey(*stm, encrypt_params, *out_stm, metadata);
                if(result == PacketResult::InvalidSurrogateIV)
                    result = PacketResult::InvalidKeyFile;
            }
            else
            {
                result = DecryptStream(*stm, encrypt_params, *out_stm, metadata);
            }
            output_buffer.resize(out_stm->GetCount());
            return result;
        }
        else // wad starts from I or P, in which the most significant bit is not set
        {
            auto result = DecryptWad(*stm, metadata.key_file, encrypt_params, *out_stm, metadata);
            output_buffer.resize(out_stm->GetCount());

            if(result == PacketResult::InvalidWadFile)
                return PacketResult::UnexpectedFormat;
            if(result == PacketResult::InvalidSurrogateIV)
                return PacketResult::InvalidKeyFile;

            metadata.key_only = true;
            return result;
        }
    }

    PacketResult DecryptPacketFile(const std::string &file_in, const std::string &file_out, 
            const EncryptParams &encrypt_params, PacketMetadata &metadata)
    {
        Buffer output_buffer;

        auto result = DecryptPacketFile(file_in, encrypt_params, output_buffer, metadata);
        if(result != PacketResult::Success)
            return result;

        result = WriteFile(file_out, output_buffer);
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
}
