#include "algo_spec.h"

namespace LibEncryptMsg
{
    const AlgoSpec &GetAlgoSpec(CipherAlgo cipher_algo)
    {
        switch(cipher_algo)
        {
            case CipherAlgo::TripleDES:
                static AlgoSpec triple_des =
                {
                    CipherAlgo::TripleDES,
                    "3DES/CFB",
                    8,
                    192
                };
                return triple_des;

            case CipherAlgo::CAST5:

                static AlgoSpec cast5 =
                {
                    CipherAlgo::CAST5,
                    "CAST5/CFB",
                    8,
                    128
                };
                return cast5;

            case CipherAlgo::AES128:

                static AlgoSpec aes128 =
                {
                    CipherAlgo::AES128,
                    "AES-128/CFB",
                    16,
                    128
                };
                return aes128;

            case CipherAlgo::AES256:

                static AlgoSpec aes256 =
                {
                    CipherAlgo::AES256,
                    "AES-256/CFB",
                    16,
                    256
                };
                return aes256;

            default:
                static AlgoSpec unknown =
                {
                    CipherAlgo::Unknown,
                    "",
                    0,
                    0
                };
                return unknown;
        }
    }

    const HashSpec &GetHashSpec(HashAlgo hash_algo)
    {
        switch(hash_algo)
        {
            case HashAlgo::SHA160:
                static HashSpec sha160 =
                {
                    HashAlgo::SHA160,
                    "OpenPGP-S2K(SHA-160)"
                };
                return sha160;
            case HashAlgo::SHA256:
                static HashSpec sha256 =
                {
                    HashAlgo::SHA256,
                    "OpenPGP-S2K(SHA-256)"
                };
                return sha256;
            case HashAlgo::SHA512:
                static HashSpec sha512 =
                {
                    HashAlgo::SHA512,
                    "OpenPGP-S2K(SHA-512)"
                };
                return sha512;
            default:
                static HashSpec unknown =
                {
                    HashAlgo::Unknown,
                    ""
                };
                return unknown;
        }
    }

    const CompressionSpec &GetCompressionSpec(Compression compression)
    {
        switch(compression)
        {
            case Compression::ZIP:
                static CompressionSpec zip =
                {
                    Compression::ZIP,
                    "deflate"
                };
                return zip;
            case Compression::ZLIB:
                static CompressionSpec zlib =
                {
                    Compression::ZLIB,
                    "zlib"
                };
                return zlib;
            case Compression::BZip2:
                static CompressionSpec bzip2 =
                {
                    Compression::BZip2,
                    "bz2"
                };
                return bzip2;
            default:
                static CompressionSpec unknown =
                {
                    Compression::Unknown,
                    ""
                };
                return unknown;
        }
    }
}

