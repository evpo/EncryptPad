#include "gtest/gtest.h"
#include <fstream>
#include <iterator>
#include "armor_reader.h"
#include "test_helper.h"

namespace EncryptMsg
{
    namespace UnitTests
    {
        class ArmorReaderFixture : public ::testing::TestWithParam<unsigned int>
        {
            protected:
                std::vector<uint8_t> plain_file_;
                std::vector<uint8_t> asc_of_plain_file_;

                virtual void SetUp() override;
        };

        static unsigned int buffer_sizes[] =
        {
            1,
            7,
            15,
            2048,
        };

        INSTANTIATE_TEST_CASE_P(Common, ArmorReaderFixture,
                ::testing::ValuesIn(buffer_sizes));

        void ArmorReaderFixture::SetUp()
        {
            LoadFile("simple_text.txt", plain_file_);
            LoadFile("simple_text.txt.plain.asc", asc_of_plain_file_);
        }

        TEST_P(ArmorReaderFixture, When_reading_armored_text_with_small_buffer_Then_output_matches)
        {
            // Arrange

            auto buf_size = GetParam();
            ArmorReader reader;
            SafeVector buf;
            SafeVector out;
            auto out_stm = MakeOutStream(out);
            EmsgResult result = EmsgResult::None;

            // Act

            auto it = asc_of_plain_file_.begin();

            while(it != asc_of_plain_file_.end())
            {
                size_t range = std::min(
                        static_cast<unsigned int>(std::distance(it, asc_of_plain_file_.end())),
                        buf_size);
                auto range_end = it;
                std::advance(range_end, range);
                buf.assign(it, range_end);
                it = range_end;
                reader.GetInStream().Push(buf);
                result = reader.Read(*out_stm);

                EXPECT_TRUE(result == EmsgResult::Success || result == EmsgResult::Pending);
            }
            auto result_after_finish = reader.Finish(*out_stm);

            // Assert

            EXPECT_EQ(EmsgResult::Success, result_after_finish);
            ASSERT_EQ(plain_file_.size(), out.size());
            ASSERT_TRUE(std::equal(out.begin(), out.end(), plain_file_.begin()));
        }

        TEST_F(ArmorReaderFixture, When_reading_armored_zero_message_Then_output_is_empty)
        {
            // Arrange

            std::vector<uint8_t> in_buf;
            LoadFile("zero.txt.asc", in_buf);
            SafeVector asc_of_zero(in_buf.begin(), in_buf.end());
            ArmorReader reader;
            SafeVector out;
            auto out_stm = MakeOutStream(out);

            // Act

            reader.GetInStream().Push(asc_of_zero);
            auto read_result = reader.Read(*out_stm);
            auto finish_result = reader.Finish(*out_stm);

            // Assert

            ASSERT_EQ(EmsgResult::Success, read_result);
            ASSERT_EQ(EmsgResult::Success, finish_result);
            ASSERT_EQ(0, out.size());
        }
    }
}
