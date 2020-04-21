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
        struct Config
        {
            size_t buffer;
            CipherAlgo cipher;
            HashAlgo hash;
        };

        class MessageEncryptionFixture : public ::testing::TestWithParam<Config> // Param is buffer size
        {
            public:
                static Config ParameterCombination[];
            protected:
                Config config_;
                vector<uint8_t> plain_file_;
                vector<uint8_t> encrypted_file_;
                secure_vector<uint8_t> buf_;

                virtual void SetUp() override;
                MessageConfig GetMessageConfig();

                SafeVector Update(MessageWriter &writer);
                SafeVector Decrypt(const SafeVector &passphrase);
        };

        Config MessageEncryptionFixture::ParameterCombination[] = {
            {buffer: 8, cipher: CipherAlgo::Twofish, hash: HashAlgo::SHA512},
            {buffer: 24, cipher: CipherAlgo::Twofish, hash: HashAlgo::SHA224},
            {buffer: 128, cipher: CipherAlgo::AES256, hash: HashAlgo::SHA160},
            {buffer: 512, cipher: CipherAlgo::AES128, hash: HashAlgo::SHA384},
            {buffer: 1024, cipher: CipherAlgo::AES128, hash: HashAlgo::SHA224},
            {buffer: 1024, cipher: CipherAlgo::CAST5, hash: HashAlgo::SHA160},
            {buffer: 1024, cipher: CipherAlgo::TripleDES, hash: HashAlgo::SHA160},
        };

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

            config_ = GetParam();
        }

        MessageConfig MessageEncryptionFixture::GetMessageConfig()
        {
            MessageConfig config;
            config.SetCipherAlgo(config_.cipher);
            config.SetHashAlgo(config_.hash);
            config.SetIterations(EncodeS2KIterations(1600000));

            config.SetCompression(Compression::ZLIB);
            config.SetFileName("encrypted_file.txt");
            config.SetFileDate(0);
            config.SetBinary(true);
            config.SetPartialLengthPower(4); // 2^4 is the partial length size. It's not legal in rfc. The length should not be less than 512
            return config;
        }

        TEST_P(MessageEncryptionFixture, When_message_writer_finishes_Then_message_matches)
        {
            //Arrange
            MessageWriter writer;
            string pwd_str("123456");
            SafeVector passphrase(FromChar(pwd_str.data()), FromChar(pwd_str.data()) + pwd_str.size());
            MessageConfig config = GetMessageConfig();

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

        TEST_P(MessageEncryptionFixture, When_encrypted_with_armor_Then_message_matches)
        {
            //Arrange
            MessageWriter writer;
            string pwd_str("123456");
            SafeVector passphrase(FromChar(pwd_str.data()), FromChar(pwd_str.data()) + pwd_str.size());
            MessageConfig config = GetMessageConfig();
            config.SetArmor(true);

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
            buf_.resize(config_.buffer);
            auto it = plain_file_.begin();
            while(it != plain_file_.end())
            {
                auto it_next = std::min(it + config_.buffer, plain_file_.end());
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
