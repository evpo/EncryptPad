#include <vector>
#include <algorithm>
#include <cstdint>
#include <string>
#include "gtest/gtest.h"
#include "test_helper.h"
#include "message_decryption.h"

namespace EncryptMsg
{
    namespace UnitTests
    {
        TEST(Assorted, When_decrypting_with_output_overflow_Then_result_is_correct)
        {
            // Arrange
            const size_t buffer_size = 8;
            MessageReader reader;
            reader.SetOutputBufferSize(64);
            std::string pwd_str("123456");
            SafeVector passphrase(FromChar(pwd_str.data()), FromChar(pwd_str.data()) + pwd_str.size());
            reader.Start(passphrase);

            std::vector<uint8_t> file;
            LoadFile("497664_zeroes.dat.bz2.gpg", file);

            // Act
            unsigned count = 0;
            SafeVector buf;
            buf.resize(buffer_size);
            auto it = file.begin();
            bool non_zero_found = false;
            while(it != file.end() || reader.OutputBufferOverflow())
            {
                auto it_next = !reader.OutputBufferOverflow() ? std::min(it + buffer_size, file.end()) : it;
                buf.resize(it_next - it);
                std::copy(it, it_next, buf.begin());
                if(it_next == file.end())
                    reader.Finish(buf);
                else
                    reader.Update(buf);
                if(!non_zero_found && std::any_of(buf.begin(), buf.end(), [](const auto &b){ return b != 0; }))
                    non_zero_found = true;

                count += buf.size();
                it = it_next;
            }

            // Assert
            ASSERT_FALSE(non_zero_found);
            ASSERT_EQ(497664, count);
        }

        TEST(Assorted, When_decrypting_bzip2_Then_result_is_correct)
        {
            // Arrange
            MessageReader reader;
            std::string pwd_str("123456");
            SafeVector passphrase(FromChar(pwd_str.data()), FromChar(pwd_str.data()) + pwd_str.size());
            reader.Start(passphrase);
            std::vector<uint8_t> file;
            std::vector<uint8_t> plain_file;
            LoadFile("simple_text.txt.bz2.gpg", file);
            LoadFile("simple_text.txt", plain_file);
            SafeVector buf {file.begin(), file.end()};

            // Act
            reader.Finish(buf);
            SafeVector tmp_buf;
            while(reader.OutputBufferOverflow())
            {
                tmp_buf.clear();
                reader.Finish(tmp_buf);
                buf.insert(buf.end(), tmp_buf.begin(), tmp_buf.end());
            }


            // Assert
            ASSERT_EQ(plain_file.size(), buf.size());
            bool result = std::equal(buf.begin(), buf.end(), plain_file.begin());
            ASSERT_TRUE(result);
        }

    }
}
