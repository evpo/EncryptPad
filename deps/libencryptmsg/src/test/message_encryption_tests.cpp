#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include "gtest/gtest.h"
#include "message_encryption.h"
#include "message_decryption.h"
#include "emsg_symmetric_key.h"
#include "openpgp_conversions.h"
#include "test_helper.h"
#include "plog/Log.h"

using namespace std;
using namespace Botan;

namespace EncryptMsg
{
    namespace UnitTests
    {
        class MessageEncryptionFixture : public ::testing::TestWithParam<size_t> // Param is buffer size
        {
            public:
                static size_t ParameterCombination[];
            protected:
                size_t buffer_size_;
                vector<uint8_t> plain_file_;
                vector<uint8_t> encrypted_file_;
                secure_vector<uint8_t> buf_;

                virtual void SetUp() override;

                SafeVector Update(MessageWriter &writer);
                SafeVector Decrypt(const SafeVector &passphrase);
        };

        size_t MessageEncryptionFixture::ParameterCombination[] = {8, 24, 128, 512, 1024};

        INSTANTIATE_TEST_CASE_P(Common, MessageEncryptionFixture,
                ::testing::ValuesIn(MessageEncryptionFixture::ParameterCombination));

        void MessageEncryptionFixture::SetUp()
        {
            const string plain_file_path = "simple_text.txt";
            const string encrypted_file_path = "simple_text.txt.gpg";

            ifstream stm(plain_file_path, ios_base::binary);
            std::copy(istreambuf_iterator<char>(stm), istreambuf_iterator<char>(),
                    back_inserter(plain_file_));
            ifstream stm2(encrypted_file_path, ios_base::binary);
            std::copy(istreambuf_iterator<char>(stm2), istreambuf_iterator<char>(),
                    back_inserter(encrypted_file_));

            buffer_size_ = GetParam();
        }

        TEST_P(MessageEncryptionFixture, When_message_writer_finishes_Then_message_is_encrypted)
        {
            //Arrange
            MessageWriter writer;
            string pwd_str("123456");
            SafeVector passphrase(FromChar(pwd_str.data()), FromChar(pwd_str.data()) + pwd_str.size());
            MessageConfig config;
            config.SetCipherAlgo(CipherAlgo::AES256);
            config.SetHashAlgo(HashAlgo::SHA256);
            config.SetIterations(EncodeS2KIterations(1600000));

            config.SetCompression(Compression::ZLIB);
            config.SetFileName("encrypted_file.txt");
            config.SetFileDate(0);
            config.SetBinary(true);
            config.SetPartialLengthPower(4); // 2^4 is the partial length size. It's not legal in rfc. The length should not be less than 512

            Salt salt = GenerateRandomSalt();

            //Act
            writer.Start(passphrase, config, salt);
            auto buf = Update(writer);

            //Assert
            buf_ = buf;
            auto output = Decrypt(passphrase);
            ASSERT_EQ(plain_file_.size(), output.size());
            bool result = std::equal(output.begin(), output.end(), plain_file_.begin());
            ASSERT_TRUE(result);
        }

        SafeVector MessageEncryptionFixture::Update(MessageWriter &writer)
        {
            SafeVector ret_val;
            buf_.resize(buffer_size_);
            auto it = plain_file_.begin();
            while(it != plain_file_.end())
            {
                auto it_next = std::min(it + buffer_size_, plain_file_.end());
                buf_.resize(it_next - it);
                std::copy(it, it_next, buf_.begin());
                if(it_next == plain_file_.end())
                    writer.Finish(buf_);
                else
                    writer.Update(buf_);
                ret_val.insert(ret_val.end(), buf_.begin(), buf_.end());
                it = it_next;
            }
            return ret_val;
        }

        SafeVector MessageEncryptionFixture::Decrypt(const SafeVector &passphrase)
        {
            MessageReader reader;
            reader.Start(passphrase);
            reader.Finish(buf_);
            return buf_;
        }
    }
}
