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
#include <ctime>
#include "assert.h"
#include "cli_parser.hpp"
#include "file_system.hpp"
#include "epad_utilities.h"
#include "file_encryption.h"
#include "key_service.h"
#include "get_passphrase.h"
#include "version.h"
#include "key_generation.h"
#include "file_helper.h"
#include "openpgp_conversions.h"
#include "algo_spec.h"

namespace EncryptPad
{
    const char *kStdIn = "-";
    const char *kStdOut = "-";

    void PrintUsage(bool use_cout = false)
    {
        const char *usage =
            VER_PRODUCTNAME_STR " " VER_PRODUCTVERSION_STR "\n"
            "\n"
            "Usage: encryptcli <command> [options] [input-file] | [key-file]\n"
            "Utility to encrypt, decrypt OpenPGP files or generate key files.\n"
            "\n"
            "Commands:\n"
            "-e|--encrypt          encrypt data\n"
            "-d|--decrypt          decrypt data\n"
            "--generate-key        generate key file\n"
            "--help                this help\n"
            "\n"
            "Parameters:\n"
            "input-file                            file to encrypt or decrypt (default: stdin)\n"
            "key-file                              generated key file in --generate-key mode (default: stdin)\n"
            "\n"
            "Options:\n"
            "-o|--output                           output file (default: stdout)\n"
            "-f|--force                            overwrite output file\n"
            "--key-file <key-file>                 key file\n"
            "--key-only                            key only, no passphrase\n"
            "--persist-key                         persist key location in the encrypted file\n"
            "--key-pwd-fd <file-descriptor>        file descriptor from which to read the key file passphrase\n"
            "--key-pwd-file <file>                 file with key passphrase\n"
            "--libcurl-path <libcurl-path>         path to libcurl executable to download a remote key file\n"
            "--force-key-pwd                       force key passphrase entry\n"
            "--plain-text-key                      plain text key (not recommended)\n"
            "--pwd-fd <file-descriptor>            passphrase file descriptor\n"
            "--pwd-file <file>                     file with passphrase\n"
            "--cipher-algo <cipher-algo>           cipher algorithm (CAST5, AES, AES256, 3DES; default: AES256)\n"
            "--compress-algo <compression-algo>    compression algorithm (ZIP, ZLIB, NONE; default: ZIP)\n"
            "--s2k-digest-algo <s2k-digest-algo>   s2k digest algorithm (SHA1, SHA256; SHA512; default: SHA256)\n"
            "--s2k-count <s2k-count>               s2k iteration count\n"
            "--key-file-length                     key file random sequence length in bytes. Use with --generate-key. default: 64\n"
            "\n"
            "Feedback: evpo.net/encryptpad\n"
            ;


        if(use_cout)
        {
            std::cout << usage << std::endl;
        }
        else
        {
            std::cerr << usage << std::endl;
        }
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
        else if(str == "SHA512")
        {
            return HashAlgo::SHA512;
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

void GenerateKeyFile(const std::string &path, size_t key_byte_size, const std::string &passphrase, PacketMetadata *kf_metadata);

// end Packet read write tests
int main(int argc, char *argv[])
{

    cli_definitions_t cli_defs = {
        {
            "encrypt",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "encrypt data",
            ""
        },
        {
            "decrypt",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "decrypt data",
            ""
        },
        {
            "generate-key",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "generate key file",
            ""
        },
        {
            "help",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "help",
            ""
        },
        {
            "output",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "output file",
            ""
        },
        {
            "force",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "overwrite output file",
            ""
        },
        {
            "key-file",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "key file",
            ""
        },
        {
            "key-only",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "key only, no passphrase",
            ""
        },
        {
            "persist-key",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "persist key location in the encrypted file",
            ""
        },
        {
            "key-pwd-fd",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "key file passphrase file descriptor",
            ""
        },
        {
            "key-pwd-file",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "key file passphrase file",
            ""
        },
        {
            "libcurl-path",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "path to libcurl executable to download remote key files",
            ""
        },
        {
            "force-key-pwd",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "request key passphrase entry",
            ""
        },
        {
            "plain-text-key",
            cli_kind_t::cli_switch_kind,
            cli_mode_t::cli_single_mode,
            "plain text key",
            ""
        },
        {
            "pwd-fd",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "passphrase file descriptor",
            ""
        },
        {
            "pwd-file",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "passphrase file",
            ""
        },
        {
            "cipher-algo",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "cipher algorithm",
            "AES256"
        },
        {
            "compress-algo",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "compression algorithm",
            "ZIP"
        },
        {
            "s2k-digest-algo",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "s2k digest algorithm",
            "SHA256"
        },
        {
            "s2k-count",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "s2k iteration count",
            ""
        },
        {
            "--key-file-length",
            cli_kind_t::cli_value_kind,
            cli_mode_t::cli_single_mode,
            "key file length",
            ""
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
    bool encrypted_kf = true;

    std::string out_file;
    std::string out_file_ext;
    std::string in_file_ext;
    std::string in_file;
    std::string passphrase_fd_str;
    std::string passphrase_file;
    int passphrase_fd = -1;
    std::string key_file;
    std::string key_file_passphrase;
    std::string key_file_passphrase_fd_str;
    std::string key_file_passphrase_file;
    int key_file_passphrase_fd = -1;
    std::string cipher_algo;
    std::string compress_algo;
    std::string hash_algo;
    int s2k_count = kDefaultIterations;
    std::string s2k_count_str;
    int key_file_length = kDefaultKeyFileKeyLength;
    std::string key_file_length_str;

    std::string libcurl_path;

    for(unsigned i = 0; i < parser.size(); i++)
    {
        if(parser.name(i) == "encrypt")
        {
            encrypt = true;
        }
        else if(parser.name(i) == "decrypt")
        {
            decrypt = true;
        }
        else if(parser.name(i) == "generate-key")
        {
            generate_kf = true;
        }
        else if(parser.name(i) == "output")
        {
            out_file = parser.string_value(i);
        }
        else if(parser.name(i) == "force")
        {
            overwrite_out_file = true;
        }
        else if(parser.name(i) == "key-file")
        {
            key_file = parser.string_value(i);
        }
        else if(parser.name(i) == "key-only")
        {
            key_only = true;
        }
        else if(parser.name(i) == "persist-key")
        {
            persist_key = true;
        }
        else if(parser.name(i) == "key-pwd-fd")
        {
            key_file_passphrase_fd_str = parser.string_value(i);
        }
        else if(parser.name(i) == "key-pwd-file")
        {
            key_file_passphrase_file = parser.string_value(i);
        }
        else if(parser.name(i) == "libcurl-path")
        {
            libcurl_path = parser.string_value(i);
        }
        else if(parser.name(i) == "force-key-pwd")
        {
            key_file_passphrase_required = true;
        }
        else if(parser.name(i) == "plain-text-key")
        {
            encrypted_kf = false;
        }
        else if(parser.name(i) == "pwd-file")
        {
            passphrase_file = parser.string_value(i);
        }
        else if(parser.name(i) == "pwd-fd")
        {
            passphrase_fd_str = parser.string_value(i);
        }
        else if(parser.name(i) == "cipher-algo")
        {
            cipher_algo = parser.string_value(i);
        }
        else if(parser.name(i) == "compress-algo")
        {
            compress_algo = parser.string_value(i);
        }
        else if(parser.name(i) == "s2k-digest-algo")
        {
            hash_algo = parser.string_value(i);
        }
        else if(parser.name(i) == "s2k-count")
        {
            s2k_count_str = parser.string_value(i);
        }
        else if(parser.name(i) == "key-file-length")
        {
            key_file_length_str = parser.string_value(i);
        }
        else if(parser.name(i) == "help")
        {
            PrintUsage(true);
            exit(1);
        }
        else if(parser.name(i) == "")
        {
            in_file = parser.string_value(i);
        }
    }

    if(!in_file.empty())
    {
        in_file_ext = extension_part(in_file);
    }

    if(in_file.empty() && !generate_kf)
    {
        in_file = kStdIn;
    }

    if(generate_kf && !in_file.empty())
    {
        out_file = in_file;
    }

    if(out_file.empty())
    {
        out_file = kStdOut;
    }
    else
    {
        out_file_ext = extension_part(out_file);
    }

    // Disable passphrase entry when gpg format and key provided
    if(encrypt && !key_only && !key_file.empty() && !out_file_ext.empty() && 
            (out_file_ext == "gpg" || out_file_ext == "GPG"))
    {
        key_only = true;
    }

    if(decrypt && !key_only && !key_file.empty() && !in_file_ext.empty() && 
            (in_file_ext == "gpg" || in_file_ext == "GPG"))
    {
        key_only = true;
    }

    if((!key_file.empty() || key_only) && encrypted_kf)
    {
        key_file_passphrase_required = true;
    }

    if(!key_file_passphrase_fd_str.empty() || !key_file_passphrase_file.empty())
    {
        key_file_passphrase_required = true;
    }

    bool invalid_args = false;

    if(generate_kf)
    {
        if(out_file.empty())
            out_file = in_file;
    }
    else if(
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

    if(!key_file_passphrase_file.empty() && !key_file_passphrase_fd_str.empty())
    {
        std::cerr << "Ambiguous arguments: both key-pwd-file and key-pwd-fd provided" << std::endl;
        exit(1);
    }

    if(!passphrase_file.empty() && !passphrase_fd_str.empty())
    {
        std::cerr << "Ambiguous arguments: both pwd-file and pwd-fd provided" << std::endl;
        exit(1);
    }

    if(!key_file_passphrase_fd_str.empty())
    {
        std::istringstream fd_stm(key_file_passphrase_fd_str);
        fd_stm >> key_file_passphrase_fd;
        if(fd_stm.fail())
        {
            std::cerr << "key-pwd-fd: '" << key_file_passphrase_fd_str << "' is not a number" << std::endl;
            exit(1);
        }

        std::string message;
        if(!ValidateFileDescriptor(key_file_passphrase_fd, message))
        {
            std::cerr << "Invalid file descriptor '" << key_file_passphrase_fd << "':"  << message << std::endl;
            exit(1);
        }
    }

    if(!s2k_count_str.empty())
    {
        std::istringstream s2k_stm(s2k_count_str);
        s2k_stm >> s2k_count;
        if(s2k_stm.fail())
        {
            std::cerr << "s2k-count: '" << s2k_count_str << "' is not a number" << std::endl;
            exit(1);
        }
        else if(s2k_count < kDefaultIterations || s2k_count > kMaxIterations)
        {
            std::cerr << "s2k-count: '" << s2k_count_str << "' is invalid" << std::endl;
            exit(1);
        }
        s2k_count = DecodeS2KIterations(EncodeS2KIterations(s2k_count));
    }

    if(!passphrase_fd_str.empty())
    {
        std::istringstream fd_stm(passphrase_fd_str);
        fd_stm >> passphrase_fd;
        if(fd_stm.fail())
        {
            std::cerr << "pwd-fd: '" << passphrase_fd_str << "' is not a number" << std::endl;
            exit(1);
        }

        std::string message;
        if(!ValidateFileDescriptor(passphrase_fd, message))
        {
            std::cerr << "Invalid file descriptor '" << passphrase_fd << "':" << message << std::endl;
            exit(1);
        }

        if(!key_file_passphrase_fd_str.empty() && passphrase_fd == key_file_passphrase_fd)
        {
            std::cerr << "The same file descriptor for pwd-fd and key-pwd-fd is not allowed" <<
                std::endl;
            exit(1);
        }
    }

    if(!out_file.empty() && out_file != kStdOut && file_exists(out_file) && !overwrite_out_file)
    {
        std::cerr << "The output file exists. Specify -f to overwrite the output file." << std::endl;
        exit(1);
    }

    if(!key_file_length_str.empty())
    {
        std::istringstream key_file_length_stm(key_file_length_str);
        key_file_length_stm >> key_file_length;
        if(key_file_length_stm.fail())
        {
            std::cerr << "key-file-length: '" << key_file_length_str << "' is not a number" << std::endl;
            exit(1);
        }
    }

    if(generate_kf)
    {
        passphrase_fd_str = key_file_passphrase_fd_str;
        passphrase_fd = key_file_passphrase_fd;
        passphrase_file = key_file_passphrase_file;
    }

    std::string passphrase;

    if(passphrase_fd_str.empty() && passphrase_file.empty() && !key_only && !(generate_kf && !encrypted_kf))
    {
        GetPassphrase("Passphrase: ", passphrase);

        if(encrypt || generate_kf)
        {
            std::string confirmed;
            GetPassphrase("Confirmation: ", confirmed);
            if(passphrase != confirmed)
            {
                std::cerr << "Passphrases don't match. Please try again.";
                exit(1);
            }
        }
    }
    else if(!key_only && !(generate_kf && !encrypted_kf))
    {
        if(!passphrase_fd_str.empty())
        {
            if(!LoadStringFromDescriptor(passphrase_fd, passphrase))
            {
                std::cerr << "Cannot read from the specified passphrase file descriptor" << std::endl;
                exit(1);
            }
        }
        else if(!passphrase_file.empty())
        {
            if(!LoadStringFromFile(passphrase_file, passphrase))
            {
                std::cerr << "Cannot read from the specified passphrase file" << std::endl;
                exit(1);
            }
        }
    }

    if(generate_kf)
    {
        try
        {
            PacketMetadata kf_metadata = GetDefaultKFMetadata(s2k_count);
            GenerateKeyFile(out_file, key_file_length, passphrase, &kf_metadata);
        }
        catch(EncryptPad::IoException &ex)
        {
            std::cerr << "Cannot generate the key file: " << ex.what() << std::endl;
            exit(1);
        }

        std::fill(passphrase.begin(), passphrase.end(), '\0');
        return 0;
    }

    if(key_file_passphrase_required && key_file_passphrase_fd_str.empty() &&
            key_file_passphrase_file.empty())
    {
        GetPassphrase("Key file passphrase: ", key_file_passphrase);
    }
    else if(!key_file_passphrase_fd_str.empty())
    {
        if(!LoadStringFromDescriptor(key_file_passphrase_fd, key_file_passphrase))
        {
            std::cerr << "Cannot read from the specified key file passphrase file descriptor" << std::endl;
            exit(1);
        }
    }
    else if(!key_file_passphrase_file.empty())
    {
        if(!LoadStringFromFile(key_file_passphrase_file, key_file_passphrase))
        {
            std::cerr << "Cannot read from the specified key file passphrase file" << std::endl;
            exit(1);
        }
    }

    if(encrypt && key_only && key_file.empty())
    {
        std::cerr << "Invalid arguments: --key-only (key only, no passphrase) requires --key-file (key file) option" << std::endl;
        PrintUsage();
        exit(1);
    }

    if(in_file != kStdIn && !file_exists(in_file))
    {
        std::cerr << "The input file does not exist" << std::endl;
        exit(1);
    }

    in_file_ext = in_file == kStdIn ? std::string() : extension_part(in_file);

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

    out_file_ext = out_file == kStdOut ? std::string() : extension_part(out_file);

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
        metadata.iterations = s2k_count;
        metadata.compression = ParseCompression(compress_algo);

        if(!metadata.key_only)
        {
            key_service.ChangePassphrase(
                    passphrase, metadata.hash_algo, GetAlgoSpec(metadata.cipher_algo).key_size, metadata.iterations);
        }

        metadata.file_name = in_file;
        metadata.file_date = static_cast<FileDate>(time(NULL));
        metadata.is_binary = true;
    }

    if(metadata.cannot_use_wad && metadata.persist_key_path)
    {
        std::cerr << "GPG format does not support persistent key path. Use EPD extension." << std::endl;
        exit(1);
    }

    if(metadata.cannot_use_wad && !passphrase.empty() && !metadata.key_only && !metadata.key_file.empty())
    {
        std::cerr << "GPG format does not support passphrase and key protection together. Use either or use EPD extension." << std::endl;
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

    if(result == PacketResult::InvalidKeyFilePassphrase && !key_file_passphrase_required)
    {
        std::cerr << "File is encrypted with an encrypted key file. Use '--force-key-pwd' switch." << std::endl;
        exit(1);
    }

    if(result != PacketResult::Success)
    {
        std::cerr << "Cannot " << (encrypt ? "encrypt" : "decrypt") << " the file: " << InterpretResult(result) << std::endl;
        exit(1);
    }
    return 0;
}

void GenerateKeyFile(const std::string &path, size_t key_byte_size, const std::string &passphrase, PacketMetadata *metadata)
{
    if(passphrase.empty())
    {
        GenerateNewKey(path, key_byte_size);
    }
    else
    {
        assert(metadata);
        EncryptParams kf_encrypt_params;
        KeyService key_service(1);
        kf_encrypt_params.key_service = &key_service;
        kf_encrypt_params.key_service->ChangePassphrase(
                    passphrase, metadata->hash_algo, GetAlgoSpec(metadata->cipher_algo).key_size,
                    metadata->iterations);
        GenerateNewKey(path, key_byte_size, &kf_encrypt_params, metadata);
    }
}
