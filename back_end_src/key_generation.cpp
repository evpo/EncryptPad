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
#include "key_generation.h"
#include "file_helper.h"
#include "key_file_converter.h"
#include "packet_composer.h"
#include "algo_spec.h"

using namespace EncryptPad;

namespace
{
    void WriteKeyFile(const std::string &key_file_path, const std::string &content)
    {
        const char *exception_msg = "Cannot write to the file";
        OutPacketStreamFile stm;
        if(OpenFile(key_file_path, stm) != OpenFileResult::OK)
            throw IoException(exception_msg);

        if(!stm.Write(reinterpret_cast<const byte*>(content.c_str()), content.length()))
            throw IoException(exception_msg);
    }

    // This implementation comes from Botan base64.cpp
    /**
     * Round up
     * @param n an integer
     * @param align_to the alignment boundary
     * @return n rounded up to a multiple of align_to
     */
    template<typename T>
        inline T botan_round_up(T n, T align_to)
        {
            if(n % align_to || n == 0)
                n += align_to - (n % align_to);
            return n;
        }
}

namespace EncryptPad
{
    void GenerateNewKey(unsigned char buffer[], size_t length)
    {
        Botan::AutoSeeded_RNG rng;
        rng.randomize(buffer, length);
    }

    void GenerateNewKey(const std::string& key_file_path, size_t key_byte_length, EncryptParams *kf_encrypt_params, PacketMetadata *metadata)
    {
        Botan::SecureVector<Botan::byte> buffer(key_byte_length);
        // The formula is from Botan base64.cpp implementation
        size_t output_buffer_size = (botan_round_up<size_t>(buffer.size(), 3) / 3) * 4;
        GenerateNewKey(buffer.begin(), buffer.size());
        Botan::SecureVector<Botan::byte> output_buffer(output_buffer_size);
        size_t input_consumed = 0U;
        size_t output_size = Botan::base64_encode(
                reinterpret_cast<char*>(output_buffer.begin()),
                buffer.begin(),
                buffer.size(),
                input_consumed,
                true);
        assert(output_size > 0 && output_size <= output_buffer.size());
        assert(input_consumed == buffer.size());
        output_buffer.resize(output_size);

        if(kf_encrypt_params && metadata)
        {
            std::string encrypted_key;
            EncryptKeyFileContent(output_buffer, kf_encrypt_params, encrypted_key, *metadata);
            WriteKeyFile(key_file_path, encrypted_key);
        }
        else
        {
            std::string plain_key(reinterpret_cast<char*>(output_buffer.begin()), output_buffer.size());
            WriteKeyFile(key_file_path, plain_key);
            std::fill(plain_key.begin(), plain_key.end(), '\0');
        }
    }

    PacketMetadata GetDefaultKFMetadata(int iterations)
    {
        PacketMetadata metadata;
        metadata.cipher_algo = kDefaultCipherAlgo;
        metadata.iterations = iterations;
        // Key file is too small for compression
        metadata.compression = Compression::Uncompressed;
        metadata.hash_algo = kDefaultHashAlgo;
        return metadata;
    }

}
