#include "gtest/gtest.h"
#include "test_helper.h"
#include "armor_writer.h"
#include "armor_reader.h"

namespace EncryptMsg
{
    namespace UnitTests
    {
        class ArmorWriterFixture : public ::testing::TestWithParam<unsigned int>
        {
            protected:
                std::vector<uint8_t> plain_file_;
                virtual void SetUp() override;
        };

        static unsigned int buffer_sizes[] = 
        {
            1,
            7,
            15,
            2048,
        };

        INSTANTIATE_TEST_CASE_P(Common, ArmorWriterFixture,
                ::testing::ValuesIn(buffer_sizes));
        void ArmorWriterFixture::SetUp()
        {
            LoadFile("simple_text.txt", plain_file_);
        }

        SafeVector Decrypt(SafeVector in)
        {
            SafeVector out;
            auto stm_out = MakeOutStream(out);
            ArmorReader reader;
            reader.GetInStream().Push(in);
            EmsgResult result = reader.Finish(*stm_out);
            assert(result == EmsgResult::Success);
            return out;
        }

        TEST_P(ArmorWriterFixture , When_writing_message_Then_decrypted_output_matches)
        {
            // Arrange

            auto buf_size = GetParam();
            ArmorWriter writer;
            SafeVector buf;
            writer.Start();

            SafeVector out;
            auto out_stm = MakeOutStream(out);

            // Act

            auto it = plain_file_.begin();
            while(it != plain_file_.end())
            {
                size_t range = std::min(
                        static_cast<unsigned int>(std::distance(it, plain_file_.end())),
                        buf_size);
                auto range_end = it;
                std::advance(range_end, range);
                buf.assign(it, range_end);
                it = range_end;
                writer.GetInStream().Push(buf);
                bool finish = (it == plain_file_.end());
                writer.Write(*out_stm, finish);
            }

            // Assert

            SafeVector decrypted_buf = Decrypt(out);
            ASSERT_EQ(plain_file_.size(), decrypted_buf.size());
            ASSERT_TRUE(std::equal(plain_file_.begin(), plain_file_.end(), decrypted_buf.begin()));
        }

        TEST_F(ArmorWriterFixture , When_writing_zero_message_Then_decrypted_output_empty)
        {
            // Arrange

            SafeVector in;
            SafeVector out;
            auto out_stm = MakeOutStream(out);
            ArmorWriter writer;
            writer.Start();

            // Act

            writer.Write(*out_stm, true);

            // Assert

            SafeVector decrypted_buf = Decrypt(out);
            ASSERT_EQ(0, decrypted_buf.size());
        }
    }
}
