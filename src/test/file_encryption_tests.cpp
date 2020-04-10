//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<evpomail@gmail.com> <http://www.evpo.net/encryptpad>
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
#include "gtest/gtest.h"
#include <cstring>
#include <string>
#include <algorithm>
#include "file_encryption.h"
#include "algo_defaults.h"

using namespace EncryptPad;
using namespace EncryptMsg;
using namespace Botan;

struct ParamItem
{
    std::string content;
    Compression compression;
    bool is_armor;
};

std::vector<ParamItem> &GetParameters()
{
    static std::vector<ParamItem> parameters =
    {
        {"test01", Compression::ZLIB, false},
        {"", Compression::ZLIB, false},
        {"da", Compression::Uncompressed, false},
        {"test01", Compression::ZLIB, true},
        {"", Compression::ZLIB, true},
    };
    return parameters;
}

// PacketStream tests
class FileEncryptionFixture : public ::testing::TestWithParam<ParamItem>
{
    protected:
        std::string passphrase_;
        PacketMetadata metadata_;
        EncryptParams enc_params_;
        KeyService key_service_;
        SecureVector<byte> in_buffer_;
        SecureVector<byte> out_buffer_;
        SecureVector<byte> result_buffer_;

        void AssignBuffer(const std::string &text)
        {
            in_buffer_.resize(text.size());
            std::copy(reinterpret_cast<const byte*>(text.data()), reinterpret_cast<const byte*>(text.data() + text.size()), in_buffer_.data());
        }

        std::string ResultString()
        {
            return std::string(
                    reinterpret_cast<const char*>(result_buffer_.data()),
                    reinterpret_cast<const char*>(result_buffer_.data() + result_buffer_.size()));
        }

        virtual void SetUp()
        {
            passphrase_ = "HwEj+1wSGpy|";
            metadata_.compression = Compression::ZLIB;
            metadata_.cipher_algo = CipherAlgo::AES256;
            metadata_.hash_algo = HashAlgo::SHA256;
            metadata_.file_name = "_CONSOLE";
            metadata_.iterations = kDefaultIterations;

            key_service_.ChangePassphrase(
                    passphrase_, metadata_.hash_algo, GetAlgoSpec(metadata_.cipher_algo).key_size, metadata_.iterations);

            enc_params_.key_service = &key_service_;
        }

        virtual void TearDown()
        {
            key_service_.Clear();
            in_buffer_.clear();
            out_buffer_.clear();
            result_buffer_.clear();
        }
};

INSTANTIATE_TEST_CASE_P(Common, FileEncryptionFixture,
        ::testing::ValuesIn(GetParameters()));

TEST_P(FileEncryptionFixture, When_text_encrypted_then_decrypted_It_is_the_same)
{
    // Prepare
    auto param_item = GetParam();
    std::string armor_head = "-----BEGIN PGP";
    AssignBuffer(param_item.content);
    metadata_.compression = param_item.compression;
    metadata_.is_armor = param_item.is_armor;

    // Act
    EpadResult encrypt_result = EncryptBuffer(in_buffer_, enc_params_, out_buffer_, metadata_);
    EpadResult decrypt_result = DecryptBuffer(out_buffer_, enc_params_, result_buffer_, metadata_);

    // Assert
    if(metadata_.is_armor)
    {
        EXPECT_TRUE(out_buffer_.size() >= armor_head.size());
        EXPECT_TRUE(std::equal(armor_head.begin(), armor_head.end(), out_buffer_.begin()));
    }
    ASSERT_EQ(EpadResult::Success, encrypt_result);
    ASSERT_EQ(EpadResult::Success, decrypt_result);
    ASSERT_EQ(param_item.content, ResultString());
}
