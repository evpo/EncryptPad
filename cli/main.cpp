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
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "assert.h"
#include "cli_parser.hpp"
#include "file_system.hpp"
#include "epad_utilities.h"
#include "file_encryption.h"
#include "key_service.h"
#include "get_password.h"
#include "version.h"
#include "key_generation.h"

namespace EncryptPad
{
    void PrintUsage()
    {
        const char *usage = 
            VER_PRODUCTNAME_STR " " VER_PRODUCTVERSION_STR "\n"
            VER_LEGALCOPYRIGHT_STR "\n"
            "\n"
            "Syntax: encrypt_cli <command> [options] <file>\n"
            "\n"
            "Commands:\n"
            "-d\t\t\tdecrypt data\n"
            "-e\t\t\tencrypt data\n"
            "--generate_kf\t\tgenerate key file\n"
            "-h, --help\t\tthis help\n"
            "\n"
            "Options:\n"
            "-o <file>\t\tuse as output file\n"
            "-p <passphrase file>\tpassphrase file\n"
            "-k <key file>\t\tkey file\n"
            "-c <cipher_algo>\tcipher algorithm (CAST5, AES, AES256, 3DES)\n"
            "-m <compression_algo>\tcompression algorithm (ZIP, ZLIB, NONE)\n"
            "-g <s2k_digest_algo>\ts2k algorithm (SHA1, SHA256)\n"
            "-n\t\t\tno passphrase, key only\n"
            "-s\t\t\tpersist key location in the encrypted file\n"
            "-f\t\t\toverwrite output file if exists\n"
            "--libcurl_path\t\tpath to libcurl executable to load remote key files\n"
            "--encrypt_kf\t\tencrypt generated key file\n"
            "--kf_passphrase_file <key file passphrase file>\t\tkey file passphrase file\n"
            "--kf_passphrase_required \t\t\t\tkey file passphrase required\n"
            ;
        std::cout << usage << std::endl;
    }

    void StringToUpper(std::string &str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }

    EncryptPad::CipherAlgo ParseCipherAlgo(std::string str)
    {
        using namespace EncryptPad;
        StringToUpper(str);
        if(str == "CAST5")
        {
            return CipherAlgo::CAST5;
        }
        else if(str == "AES")
        {
            return CipherAlgo::AES128;
        }
        else if(str == "AES256")
        {
            return CipherAlgo::AES256;
        }
        else if(str == "3DES")
        {
            return CipherAlgo::TripleDES;
        }
        else
        {
            return kDefaultCipherAlgo;
        }
    }

    EncryptPad::HashAlgo ParseHashAlgo(std::string str)
    {
        using namespace EncryptPad;
        StringToUpper(str);
        if(str == "SHA1")
        {
            return HashAlgo::SHA160;
        }
        else if(str == "SHA256")
        {
            return HashAlgo::SHA256;
        }
        else
        {
            return kDefaultHashAlgo;
        }
    }

    EncryptPad::Compression ParseCompression(std::string str)
    {
        using namespace EncryptPad;
        StringToUpper(str);
        if(str == "ZIP")
        {
            return Compression::ZIP;
        }
        else if(str == "ZLIB")
        {
            return Compression::ZLIB;
        }
        else if(str == "NONE")
        {
            return Compression::Uncompressed;
        }
        else
        {
            return kDefaultCompression;
        }
    }
}

using namespace EncryptPad;
using namespace stlplus;

void GenerateKeyFile(const std::string &path, const std::string &passphrase);

// end Packet read write tests
int main(int argc, char *argv[])
{

    cli_definitions_t cli_defs = {
        {
            "d",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "decrypt data",
            ""
        },
        {
            "e",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "encrypt data",
            ""
        },
        {
            "-help",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "-help",
            ""
        },
        {
            "f",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "overwrite output file",
            ""
        },
        {
            "n",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "no passphrase, key only",
            ""
        },
        {
            "s",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "persist key location in the encrypted file",
            ""
        },
        {
            "o",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "use as output file",
            ""
        },
        {
            "p",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "passphrase file",
            ""
        },
        {
            "k",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "key file",
            ""
        },
        {
            "-libcurl_path",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "path to libcurl executable to download remote key files",
            ""
        },
        {
            "-kf_passphrase_file",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "key file passphrase file",
            ""
        },
        {
            "-kf_passphrase_required",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "key file passphrase required",
            ""
        },
        {
            "-generate_kf",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "generate key file",
            ""
        },
        {
            "-encrypt_kf",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "encrypt key file",
            ""
        },
        {
            "c",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "cipher algorithm",
            "AES256"
        },
        {
            "m",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "compression algorithm",
            "ZIP"
        },
        {
            "g",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "s2k digest algorithm",
            "SHA256"
        },
        {
            "",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "input file",
            ""
        },
        END_CLI_DEFINITIONS,
    };

    message_handler messages(std::cerr);

    cli_parser parser(&cli_defs[0], messages);

    if(!parser.parse(argv))
    {
        PrintUsage();
        exit(1);
    }

    bool encrypt = false;
    bool decrypt = false;
    bool generate_kf = false;

    bool overwrite_out_file = false;
    bool key_only = false;
    bool persist_key = false;
    bool key_file_passphrase_required = false;
    bool encrypt_kf = false;

    std::string out_file;
    std::string in_file;
    std::string passphrase_file;
    std::string key_file;
    std::string key_file_passphrase;
    std::string key_file_passphrase_file;
    std::string cipher_algo;
    std::string compress_algo;
    std::string hash_algo;

    std::string libcurl_path;

    for(unsigned i = 0; i < parser.size(); i++)
    {
        if(parser.name(i) == "e")
        {
            encrypt = true;
        }
        else if(parser.name(i) == "d")
        {
            decrypt = true;
        }
        else if(parser.name(i) == "-generate_kf")
        {
            generate_kf = true;
        }
        else if(parser.name(i) == "f")
        {
            overwrite_out_file = true;
        }
        else if(parser.name(i) == "-encrypt_kf")
        {
            encrypt_kf = true;
        }
        else if(parser.name(i) == "n")
        {
            key_only = true;
        }
        else if(parser.name(i) == "s")
        {
            persist_key = true;
        }
        else if(parser.name(i) == "-kf_passphrase_required")
        {
            key_file_passphrase_required = true;
        }
        else if(parser.name(i) == "o")
        {
            out_file = parser.string_value(i);
        }
        else if(parser.name(i) == "p")
        {
            passphrase_file = parser.string_value(i);
        }
        else if(parser.name(i) == "k")
        {
            key_file = parser.string_value(i);
        }
        else if(parser.name(i) == "-kf_passphrase_file")
        {
            key_file_passphrase_file = parser.string_value(i);
        }
        else if(parser.name(i) == "c")
        {
            cipher_algo = parser.string_value(i);
        }
        else if(parser.name(i) == "m")
        {
            compress_algo = parser.string_value(i);
        }
        else if(parser.name(i) == "g")
        {
            hash_algo = parser.string_value(i);
        }
        else if(parser.name(i) == "-libcurl_path")
        {
            libcurl_path = parser.string_value(i);
        }
        else if(parser.name(i) == "-help")
        {
            PrintUsage();
            exit(1);
        }
        else if(parser.name(i) == "")
        {
            in_file = parser.string_value(i);
        }
    }

    bool invalid_args = false;

    if(generate_kf)
    {
        if(out_file.empty())
            out_file = in_file;

        if(out_file.empty())
        {
            invalid_args = true;
        }
    }
    else if(
        in_file.empty() ||
        (encrypt && decrypt) ||
        (!encrypt && !decrypt)
        )
    {
        invalid_args = true;
    }

    if(invalid_args)
    {
        PrintUsage();
        exit(1);
    }

    if(file_exists(out_file) && !overwrite_out_file)
    {
        std::cout << "The output file exists. Specify -f to overwrite the output file." << std::endl;
        exit(1);
    }

    if(generate_kf)
    {
        passphrase_file = key_file_passphrase_file;
    }

    std::string passphrase;
    if(passphrase_file.empty() && !key_only && !(generate_kf && !encrypt_kf))
    {
        std::cout << "Password: ";
        GetPassword(passphrase);

        if(encrypt || generate_kf)
        {
            std::cout << "Confirm: ";
            std::string confirmed;
            GetPassword(confirmed);
            if(passphrase != confirmed)
            {
                std::cerr << "Passwords don't match. Please try again.";
                exit(1);
            }
        }
    }
    else if(!key_only && !(generate_kf && !encrypt_kf))
    {
        if(!LoadStringFromFile(passphrase_file, passphrase))
        {
            std::cerr << "Cannot read from the specified passphrase file" << std::endl;
            exit(1);
        }
    }

    if(generate_kf)
    {
        try
        {
            GenerateKeyFile(out_file, passphrase);
        }
        catch(EncryptPad::IoException &ex)
        {
            std::cerr << "Cannot generate the key file: " << ex.what() << std::endl;
            exit(1);
        }

        std::fill(passphrase.begin(), passphrase.end(), '\0');
        return 0;
    }

    if(key_file_passphrase_required && key_file_passphrase_file.empty())
    {
        std::cout << "Key file password: ";
        GetPassword(key_file_passphrase);
    }
    else if(!key_file_passphrase_file.empty())
    {
        if(!LoadStringFromFile(key_file_passphrase_file, key_file_passphrase))
        {
            std::cerr << "Cannot read from the specified key file password file" << std::endl;
            exit(1);
        }
    }

    if(encrypt && key_only && key_file.empty())
    {
        std::cerr << "Invalid arguments: -n (key only, no password) requires -k (key file) option" << std::endl;
        PrintUsage();
        exit(1);
    }

    if(!file_exists(in_file))
    {
        std::cerr << "The input file does not exist" << std::endl;
        exit(1);
    }

    std::string in_file_ext = extension_part(in_file);

    if(encrypt)
    {
        if(out_file.empty())
            out_file = in_file + ".epd";
    }
    else if(decrypt)
    {
        if(out_file.empty())
        {
            if(in_file_ext != "gpg")
            {
                std::cerr << "Cannot infer the output file name from the input file name. -o option is required." << std::endl;
                exit(1);
            }

            out_file = basename_part(in_file);
        }
    }

    std::string out_file_ext = extension_part(out_file);


    EncryptParams key_file_encrypt_params = {};
    KeyService key_file_key_service(1);
    key_file_encrypt_params.key_service = &key_file_key_service;
    key_file_encrypt_params.passphrase = &key_file_passphrase;

    EncryptParams enc_params = {};
    KeyService key_service(1);

    PacketMetadata metadata = {};
    metadata.key_only = key_only;
    metadata.key_file = key_file;
    metadata.persist_key_path = persist_key;
    metadata.cannot_use_wad = (encrypt && out_file_ext == "gpg") || (decrypt && in_file_ext == "gpg");

    if(encrypt)
    {
        metadata.hash_algo = ParseHashAlgo(hash_algo);
        metadata.cipher_algo = ParseCipherAlgo(cipher_algo);
        metadata.iterations = kDefaultIterations;
        metadata.compression = ParseCompression(compress_algo);

        if(!metadata.key_only)
        {
            key_service.ChangePassphrase(
                    passphrase, metadata.hash_algo, GetAlgoSpec(metadata.cipher_algo).key_size);
        }

        metadata.file_name = in_file;
        metadata.file_date = 0;
        metadata.is_binary = true;
    }

    if(metadata.cannot_use_wad && metadata.persist_key_path)
    {
        std::cout << "GPG format does not support persisted key path. Use EPD extension." << std::endl;
        exit(1);
    }

    if(metadata.cannot_use_wad && !passphrase.empty() && !metadata.key_only && !metadata.key_file.empty())
    {
        std::cout << "GPG format does not support password and key protection together. Use either or use EPD extension." << std::endl;
        exit(1);
    }

    enc_params.libcurl_path = &libcurl_path;
    enc_params.key_service = &key_service;
    // for encryption we'll take the generated salt and key. This lets us save without knowing the passphrase
    enc_params.passphrase = encrypt ? nullptr : &passphrase;

    if(!key_file_passphrase.empty())
    {
        enc_params.key_file_encrypt_params = &key_file_encrypt_params;
    }
        
    PacketResult result = PacketResult::None;
    if(encrypt)
    {
        result = EncryptPacketFile(in_file, out_file, enc_params, metadata);
    }
    else if(decrypt)
    {
        result = DecryptPacketFile(in_file, out_file, enc_params, metadata);
    }

    if(result != PacketResult::Success)
    {
        std::cerr << "Cannot " << (encrypt ? "encrypt" : "decrypt") << " the file: " << InterpretResult(result) << std::endl;
        exit(1);
    }
    return 0;
}

void GenerateKeyFile(const std::string &path, const std::string &passphrase)
{
    if(passphrase.empty())
    {
        GenerateNewKey(path);
    }
    else
    {
        EncryptParams kf_encrypt_params;
        KeyService key_service(1);
        kf_encrypt_params.key_service = &key_service;
        PacketMetadata metadata = GetDefaultKFMetadata();
        kf_encrypt_params.key_service->ChangePassphrase(
                    passphrase, metadata.hash_algo, GetAlgoSpec(metadata.cipher_algo).key_size);
        GenerateNewKey(path, &kf_encrypt_params, &metadata);
    }
}
