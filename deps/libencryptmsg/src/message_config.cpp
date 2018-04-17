//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "message_config.h"
#include "emsg_constants.h"
#include "openpgp_conversions.h"

namespace EncryptMsg
{
    CipherAlgo MessageConfig::GetCipherAlgo() const
    {
        return cipher_algo_;
    }
    void MessageConfig::SetCipherAlgo(CipherAlgo algo)
    {
        cipher_algo_ = algo;
    }

    HashAlgo MessageConfig::GetHashAlgo() const
    {
        return hash_algo_;
    }
    void MessageConfig::SetHashAlgo(HashAlgo hash_algo)
    {
        hash_algo_ = hash_algo;
    }

    uint8_t MessageConfig::GetIterations() const
    {
        return iterations_;
    }
    void MessageConfig::SetIterations(uint8_t iterations)
    {
        iterations_ = iterations;
    }

    Compression MessageConfig::GetCompression() const
    {
        return compression_;
    }
    void MessageConfig::SetCompression(Compression compression)
    {
        compression_ = compression;
    }

    const std::string &MessageConfig::GetFileName() const
    {
        return file_name_;
    }
    void MessageConfig::SetFileName(const std::string &file_name)
    {
        file_name_ = file_name;
    }

    FileDate MessageConfig::GetFileDate() const
    {
        return file_date_;
    }
    void MessageConfig::SetFileDate(FileDate file_date)
    {
        file_date_ = file_date;
    }

    // Binary or text
    bool MessageConfig::GetBinary() const
    {
        return binary_;
    }
    void MessageConfig::SetBinary(bool binary)
    {
        binary_ = binary;
    }

    // Power of 2 to specify the length of segments
    uint8_t MessageConfig::GetPartialLengthPower() const
    {
        return partial_length_power_;
    }
    void MessageConfig::SetPartialLengthPower(uint8_t power)
    {
        partial_length_power_ = power;
    }

    MessageConfig::MessageConfig():
            cipher_algo_(kDefaultCipherAlgo),
            hash_algo_(kDefaultHashAlgo),
            iterations_(EncodeS2KIterations(kDefaultIterations)),
            compression_(kDefaultCompression),
            file_name_(""),
            file_date_(0),
            binary_(kDefaultBinary),
            partial_length_power_(kDefaultPartialLengthPower)
    {
    }
}

