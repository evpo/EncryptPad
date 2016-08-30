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
#include "botan.h"
#include "file_helper.h"
#include "key_file_converter.h"
#include "packet_composer.h"

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
}

namespace EncryptPad
{
	void GenerateNewKey(unsigned char buffer[], size_t length)
	{
		Botan::AutoSeeded_RNG rng;
		rng.randomize(buffer, length);
	}

    void GenerateNewKey(const std::string& key_file_path, EncryptParams *kf_encrypt_params, PacketMetadata *metadata)
	{
		const size_t key_byte_length = 256 >> 3; // converts bits into bytes
		Botan::byte buffer[key_byte_length]; // calculate the number of bytes to represent 256 bit
		GenerateNewKey(buffer, key_byte_length);

        std::string key_string = Botan::base64_encode(buffer, key_byte_length);
        if(kf_encrypt_params && metadata)
        {
            EncryptKeyFileContent(key_string, kf_encrypt_params, key_string, *metadata);
        }

        WriteKeyFile(key_file_path, key_string);
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
