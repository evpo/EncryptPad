//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <cstdint>
#include "algo_spec.h"
#include "emsg_types.h"
#include "emsg_utility.h"
#include "utility.h"

namespace EncryptMsg
{
    class MessageConfig
    {
        private:
            CipherAlgo cipher_algo_;
            HashAlgo hash_algo_;
            uint8_t iterations_;
            Compression compression_;
            std::string file_name_;
            FileDate file_date_;
            bool binary_;
            uint8_t partial_length_power_;
        public:
            PUBIF MessageConfig();
            PUBIF CipherAlgo GetCipherAlgo() const;
            PUBIF void SetCipherAlgo(CipherAlgo algo);

            PUBIF HashAlgo GetHashAlgo() const;
            PUBIF void SetHashAlgo(HashAlgo hash_algo);

            PUBIF uint8_t GetIterations() const;
            PUBIF void SetIterations(uint8_t iterations);

            PUBIF Compression GetCompression() const;
            PUBIF void SetCompression(Compression compression);

            PUBIF const std::string &GetFileName() const;
            PUBIF void SetFileName(const std::string &file_name);

            PUBIF FileDate GetFileDate() const;
            PUBIF void SetFileDate(FileDate file_date);

            // Binary or text
            PUBIF bool GetBinary() const;
            PUBIF void SetBinary(bool binary);

            // Power of 2 to specify the length of segments
            PUBIF uint8_t GetPartialLengthPower() const;
            PUBIF void SetPartialLengthPower(uint8_t power);
    };

}

