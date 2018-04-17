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
            MessageConfig();
            CipherAlgo GetCipherAlgo() const;
            void SetCipherAlgo(CipherAlgo algo);

            HashAlgo GetHashAlgo() const;
            void SetHashAlgo(HashAlgo hash_algo);

            uint8_t GetIterations() const;
            void SetIterations(uint8_t iterations);

            Compression GetCompression() const;
            void SetCompression(Compression compression);

            const std::string &GetFileName() const;
            void SetFileName(const std::string &file_name);

            FileDate GetFileDate() const;
            void SetFileDate(FileDate file_date);

            // Binary or text
            bool GetBinary() const;
            void SetBinary(bool binary);

            // Power of 2 to specify the length of segments
            uint8_t GetPartialLengthPower() const;
            void SetPartialLengthPower(uint8_t power);
    };

}

