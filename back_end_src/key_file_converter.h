#pragma once
#include <string>
#include "packet_composer.h"

namespace EncryptPad
{
    // Decrypt encrypted key file content
    // key is just copied to out if it is not encrypted
    // returns true if the decryption is successful
    bool DecryptKeyFileContent(const std::string &key, EncryptParams *key_file_encrypt_params, std::string &out);

    // Encrypts key file content
    bool EncryptKeyFileContent(Botan::SecureVector<Botan::byte> &in_buffer, EncryptParams *key_file_encrypt_params, std::string &out,
            PacketMetadata &metadata);
}
