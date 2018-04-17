//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************

#pragma once
#include <string>

namespace EncryptMsg
{
    enum class Compression
    {
        Unknown = -1,
        Uncompressed = 0,
        ZIP = 1,
        ZLIB = 2,
        BZip2 = 3,
    };

    enum class CipherAlgo
    {
        Unknown = -1,
        TripleDES = 2,
        CAST5 = 3,
        AES128 = 7,
        AES256 = 9,
    };

    enum class HashAlgo
    {
        Unknown = -1,
        SHA160 = 2,
        SHA256 = 8,
        SHA512 = 10,
    };

    struct AlgoSpec
    {
        CipherAlgo cipher_algo;
        std::string botan_name;
        unsigned block_size;
        unsigned key_size;
    };

    struct HashSpec
    {
        HashAlgo hash_algo;
        std::string botan_name;
    };

    struct CompressionSpec
    {
        Compression compression;
        std::string botan_name;
    };

    const AlgoSpec &GetAlgoSpec(CipherAlgo cipher_algo);
    const HashSpec &GetHashSpec(HashAlgo hash_algo);
    const CompressionSpec &GetCompressionSpec(Compression compression);
}
