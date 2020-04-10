#include <algorithm>
#include <array>
#include "gtest/gtest.h"
#include "plog/Log.h"
#include "memory_stream.h"

using namespace std;
using namespace Botan;

namespace EncryptMsg
{
    namespace UnitTests
    {
        using SafeVector = Botan::secure_vector<uint8_t>;
        const uint8_t kLen16 = 0xE4;

        class InputMemoryStreamFixture : public ::testing::Test
        {
        };


        TEST_F(InputMemoryStreamFixture, When_length_is_incomplete_Then_stream_continues_when_length_is_available)
        {
            // Arrange
            array<uint8_t, 5> long_length = {0xFF,0x00,0x00,0x20,0xC0}; // 8384
            SafeVector buf;
            buf.push_back(kLen16); // 16 bytes
            buf.insert(buf.end(), 16, 0x01);
            InBufferStream stm;
            stm.SetPartialLength(true);

            //Act
            stm.Push(buf);
            buf.clear();
            SafeVector out_buf_1(stm.GetCount());
            stm.Read(out_buf_1.data(), out_buf_1.size());
            // The stream has emptied the buffer and it is waiting for the next block length
            auto length_1 = stm.GetCount();
            // push 3 bytes from long_length
            buf.insert(buf.end(), long_length.begin(), long_length.begin() + 3);
            stm.Push(buf);
            buf.clear();
            auto length_2 = stm.GetCount();
            // finish the length
            buf.insert(buf.end(), long_length.begin() + 3, long_length.end());
            stm.Push(buf);
            buf.clear();
            auto length_3 = stm.GetCount();
            // now push some data
            buf.insert(buf.end(), 8, 0x02);
            stm.Push(buf);
            buf.clear();
            SafeVector out_buf_2(stm.GetCount());
            stm.Read(out_buf_2.data(), out_buf_2.size());

            //Assert
            ASSERT_EQ(0U, length_1);
            ASSERT_EQ(16U, out_buf_1.size());
            ASSERT_TRUE(std::all_of(out_buf_1.begin(), out_buf_1.end(), [](uint8_t e){ return e == 0x01; }));
            ASSERT_EQ(0U, length_2);
            ASSERT_EQ(0U, length_3);
            ASSERT_EQ(8U, out_buf_2.size());
            ASSERT_TRUE(std::all_of(out_buf_2.begin(), out_buf_2.end(), [](uint8_t e){ return e == 0x02; }));
        }

        TEST_F(InputMemoryStreamFixture, When_reading_buffer_Then_stream_just_works)
        {
            // Arrange
            SafeVector buf;
            buf.push_back(kLen16); // 16 bytes
            buf.insert(buf.end(), 16, 0x01);
            buf.push_back(kLen16); // 16 bytes
            buf.insert(buf.end(), 8, 0x01); // 8 bytes need to come

            InBufferStream stm;
            stm.SetPartialLength(true);
            stm.Push(buf);

            // Act
            size_t first_count = stm.GetCount();
            buf.clear();
            buf.insert(buf.end(), 8, 0x02);
            stm.Push(buf);
            size_t second_count = stm.GetCount();
            buf.clear();
            buf.resize(stm.GetCount());
            stm.Read(buf.data(), buf.size());

            // Assert
            ASSERT_EQ(24U, first_count);
            ASSERT_EQ(32U, second_count);
            ASSERT_TRUE(std::all_of(buf.begin(), buf.begin() + 24, [](int e){ return e == 0x01; }));
            ASSERT_TRUE(std::all_of(buf.begin() + 24, buf.end(), [](int e){ return e == 0x02; }));
        }

        TEST_F(InputMemoryStreamFixture, When_reading_without_partial_length_Then_it_just_works)
        {
            // Arrange
            SafeVector buf;
            buf.insert(buf.end(), 16, 0x01);
            InBufferStream stm;
            stm.Push(buf);

            // Act
            size_t first_count = stm.GetCount();
            buf.clear();
            SafeVector out_buf_1(stm.GetCount());
            stm.Read(out_buf_1.data(), out_buf_1.size());

            buf.insert(buf.end(), 5, 0x02);
            stm.Push(buf);
            buf.clear();
            size_t second_count = stm.GetCount();
            SafeVector out_buf_2(stm.GetCount());
            stm.Read(out_buf_2.data(), out_buf_2.size());


            // Assert
            ASSERT_EQ(16U, first_count);
            ASSERT_TRUE(std::all_of(out_buf_1.begin(), out_buf_1.end(), [](int e){ return e == 0x01; }));

            ASSERT_EQ(5U, second_count);
            ASSERT_TRUE(std::all_of(out_buf_2.begin(), out_buf_2.end(), [](int e){ return e == 0x02; }));
        }
    }
}
