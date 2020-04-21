#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include "gtest/gtest.h"
#include "message_decryption.h"
#include "openpgp_conversions.h"
#include "test_helper.h"
#include "plog/Log.h"

using namespace std;
using namespace Botan;

namespace EncryptMsg
{
    namespace UnitTests
    {
        struct TestParameters
        {
            size_t buffer_size;
            string encrypted_file;
            size_t iterations;
            string salt;
        };

        class MessageDecryptionFixture : public ::testing::TestWithParam<TestParameters>
        {
            public:
                static TestParameters ParameterCombination[];
            protected:
                TestParameters parameters_;
                size_t buffer_size_;
                vector<uint8_t> plain_file_;
                vector<uint8_t> encrypted_file_;
                SafeVector buf_;

                virtual void SetUp() override;

                void TestSecurity(const MessageConfig &config, const Salt &salt);
                bool Update(PacketAnalyzer &analyzer, bool stop_when_analyzed = false);
                SafeVector Update(MessageReader &reader, std::vector<uint8_t> &file);
        };

        TestParameters MessageDecryptionFixture::ParameterCombination[] =
        {
            {1024, "simple_text.txt.twofish.sha160.gpg", 37748736, "B20F6271BFD9FAE3"},
            {1024, "simple_text.txt.aes192.sha160.gpg", 37748736, "5E91968C5DE5CE34"},
            {1024, "simple_text.txt.aes256.sha512.gpg", 37748736, "02F89FB223F53ED5"},
            {1024, "simple_text.txt.aes256.sha384.gpg", 37748736, "DD6C4862CEE8586B"},
            {1024, "simple_text.txt.aes256.sha224.gpg", 37748736, "2EBD66CAD3F95805"},
            {8, "simple_text.txt.aes256.sha256.gpg", 1015808, "04D58C5C071A5B3F"},
            {24, "simple_text.txt.aes256.sha256.gpg", 1015808, "04D58C5C071A5B3F"},
            {128, "simple_text.txt.aes256.sha256.gpg", 1015808, "04D58C5C071A5B3F"},
            {512, "simple_text.txt.aes256.sha256.gpg", 1015808, "04D58C5C071A5B3F"},
            {1024, "simple_text.txt.aes256.sha256.gpg", 1015808, "04D58C5C071A5B3F"},
            {8, "simple_text.txt.cast5.sha160.gpg", 65536, "E688F0DA9651F2F6"},
            {24, "simple_text.txt.cast5.sha160.gpg", 65536, "E688F0DA9651F2F6"},
            {128, "simple_text.txt.cast5.sha160.gpg", 65536, "E688F0DA9651F2F6"},
            {512, "simple_text.txt.cast5.sha160.gpg", 65536, "E688F0DA9651F2F6"},
            {1024, "simple_text.txt.cast5.sha160.gpg", 65536, "E688F0DA9651F2F6"},
            {8, "simple_text.txt.aes128.sha160.asc", 35651584, "5FF41A59F11682F1"},
            {24, "simple_text.txt.aes128.sha160.asc", 35651584, "5FF41A59F11682F1"},
            {128, "simple_text.txt.aes128.sha160.asc", 35651584, "5FF41A59F11682F1"},
            {512, "simple_text.txt.aes128.sha160.asc", 35651584, "5FF41A59F11682F1"},
            {1024, "simple_text.txt.aes128.sha160.asc", 35651584, "5FF41A59F11682F1"},
        };

        INSTANTIATE_TEST_CASE_P(Common, MessageDecryptionFixture,
                ::testing::ValuesIn(MessageDecryptionFixture::ParameterCombination));

        void MessageDecryptionFixture::SetUp()
        {
            const string plain_file_path = "simple_text.txt";
            parameters_ = GetParam();
            buffer_size_ = parameters_.buffer_size;

            ifstream stm(plain_file_path, ios_base::binary);
            std::copy(istreambuf_iterator<char>(stm), istreambuf_iterator<char>(),
                    back_inserter(plain_file_));
            ifstream stm2(parameters_.encrypted_file, ios_base::binary);
            std::copy(istreambuf_iterator<char>(stm2), istreambuf_iterator<char>(),
                    back_inserter(encrypted_file_));

        }

        TEST_P(MessageDecryptionFixture, When_packet_analyzer_without_passphrase_Then_essentials_returned)
        {
            //Arrange
            PacketAnalyzer analyzer;

            //Act
            analyzer.Start();
            bool is_analyzed = Update(analyzer, true);

            //Assert
            ASSERT_TRUE(is_analyzed);
            auto &config = analyzer.GetMessageConfig();
            TestSecurity(config, analyzer.GetSalt());
        }

        TEST_P(MessageDecryptionFixture, When_message_reader_finishes_Then_message_is_decrypted)
        {
            //Arrange
            MessageReader reader;
            string pwd_str("123456");
            SafeVector passphrase(FromChar(pwd_str.data()), FromChar(pwd_str.data()) + pwd_str.size());

            //Act
            reader.Start(passphrase);
            auto output = Update(reader, encrypted_file_);

            //Assert
            ASSERT_EQ(plain_file_.size(), output.size());
            bool result = std::equal(output.begin(), output.end(), plain_file_.begin());
            ASSERT_TRUE(result);

        }

        TEST_P(MessageDecryptionFixture, When_packet_analyzer_works_Then_headers_are_correct)
        {
            //Arrange
            PacketAnalyzer analyzer;
            string pwd_str("123456");
            SafeVector passphrase(FromChar(pwd_str.data()), FromChar(pwd_str.data()) + pwd_str.size());

            //Act
            analyzer.Start(passphrase);
            bool is_analyzed = Update(analyzer, true);

            //Assert
            ASSERT_TRUE(is_analyzed);
            auto &config = analyzer.GetMessageConfig();
            TestSecurity(config, analyzer.GetSalt());
            ASSERT_EQ(Compression::ZIP, config.GetCompression());
            ASSERT_EQ("simple_text.txt", config.GetFileName());
            ASSERT_EQ(true, config.GetBinary());
        }

        std::string NextToken(std::string &text)
        {
            auto it = std::find(text.begin(), text.end(), '.');
            if(it == text.end())
                return std::string();

            std::string ret_val(text.begin(), it);
            it++;
            text.erase(text.begin(), it);
            return ret_val;
        }

        void MessageDecryptionFixture::TestSecurity(const MessageConfig &config, const Salt &salt)
        {
            std::string file_name = parameters_.encrypted_file;

            // remove "simple_text.txt."
            NextToken(file_name);
            NextToken(file_name);
            std::string cipher = NextToken(file_name);
            std::string hash = NextToken(file_name);
            std::string ext = NextToken(file_name);

            // Cipher
            if(cipher == "cast5")
            {
                ASSERT_EQ(CipherAlgo::CAST5, config.GetCipherAlgo());
            }
            else if(cipher == "aes192")
            {
                ASSERT_EQ(CipherAlgo::AES192, config.GetCipherAlgo());
            }
            else if(cipher == "aes256")
            {
                ASSERT_EQ(CipherAlgo::AES256, config.GetCipherAlgo());
            }
            else if(cipher == "twofish")
            {
                ASSERT_EQ(CipherAlgo::Twofish, config.GetCipherAlgo());
            }

            // Hash
            if(hash == "sha160")
            {
                ASSERT_EQ(HashAlgo::SHA160, config.GetHashAlgo());
            }
            else if(hash == "sha256")
            {
                ASSERT_EQ(HashAlgo::SHA256, config.GetHashAlgo());
            }

            // Armor
            if(ext == "asc")
            {
                ASSERT_TRUE(config.GetArmor());
            }
            else if(ext == "gpg")
            {
                ASSERT_FALSE(config.GetArmor());
            }

            ASSERT_EQ(parameters_.iterations, DecodeS2KIterations(config.GetIterations()));
            ostringstream stm;
            stm << std::hex << std::setfill('0') << std::uppercase;
            auto salt_it = salt.begin();
            for(;salt_it != salt.end(); salt_it++)
            {
                stm << std::setw(2) << static_cast<unsigned>(*salt_it);
            }
            string salt_str = stm.str();
            ASSERT_EQ(parameters_.salt, salt_str);
        }

        SafeVector MessageDecryptionFixture::Update(MessageReader &reader, std::vector<uint8_t> &file)
        {
            SafeVector ret_val;
            buf_.resize(buffer_size_);
            auto it = file.begin();
            while(it != file.end())
            {
                auto it_next = std::min(it + buffer_size_, file.end());
                buf_.resize(it_next - it);
                std::copy(it, it_next, buf_.begin());
                if(it_next == file.end())
                    reader.Finish(buf_);
                else
                    reader.Update(buf_);
                ret_val.insert(ret_val.end(), buf_.begin(), buf_.end());
                it = it_next;
            }
            return ret_val;
        }

        bool MessageDecryptionFixture::Update(PacketAnalyzer &analyzer, bool stop_when_analyzed)
        {
            buf_.resize(buffer_size_);
            auto it = encrypted_file_.begin();
            bool is_analyzed = false;
            while(it != encrypted_file_.end() && (!is_analyzed || !stop_when_analyzed))
            {
                auto it_next = std::min(it + buffer_size_, encrypted_file_.end());
                buf_.resize(it_next - it);
                std::copy(it, it_next, buf_.begin());
                is_analyzed = analyzer.Update(buf_);
                it = it_next;
            }

            buf_.clear();

            if(stop_when_analyzed && is_analyzed)
            {
                return is_analyzed;
            }
            else
            {
                is_analyzed = analyzer.Finish(buf_);
            }

            return is_analyzed;
        }
    }
}
