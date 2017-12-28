//**********************************************************************************
//EncryptPad Copyright 2015 Evgeny Pokhilko 
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
#include <cstring>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include "gtest/gtest.h"
#include "botan.h"
#include "packet_stream.h"

using namespace EncryptPad;

// PacketStream tests
class InPacketStreamFixture : public ::testing::Test
{
protected:
    Botan::SecureVector<byte> buffer_;
    InPacketStreamMemory in_;
    int length_;

    virtual void SetUp()
    {
        const char *str = "Hello 123456";
        length_ = strlen(str) + 1;
        buffer_.resize(length_);
        std::copy_n(reinterpret_cast<const byte*>(str), length_, buffer_.data());
        in_.Set(buffer_.data(), buffer_.data() + buffer_.size());
    }

    virtual void TearDown()
    {
        buffer_.clear();
    }
};

TEST_F(InPacketStreamFixture, After_it_opens_SecureVector_It_gets_bytes)
{
    // Arrange
    std::ostringstream ostm;

    // Act
    
    bool eof_before = in_.IsEOF();
    std::generate_n(std::ostream_iterator<char>(ostm), length_ - 1, [this](){return in_.Get();});
    int count_before_last = in_.GetCount();
    int last_byte = in_.Get();
    int eof1 = in_.Get();
    int eof2 = in_.Get();
    bool eof_after = in_.IsEOF();

    // Assert
    ASSERT_FALSE(eof_before);
    ASSERT_EQ(1, count_before_last);
    ASSERT_EQ(0, in_.GetCount());
    ASSERT_EQ("Hello 123456", ostm.str());
    ASSERT_EQ(0, last_byte);
    ASSERT_EQ(-1, eof1);
    ASSERT_EQ(-1, eof2);
    ASSERT_TRUE(eof_after);
}

TEST_F(InPacketStreamFixture, After_it_opens_SecureVector_It_reads_bytes)
{
    //Arrange
    const char *expected_str = "Hello 123456";
    std::vector<char> data;
    data.resize(strlen(expected_str) + 1);
    auto out_it = data.data();

    //Act
    int read_bytes1 = in_.Read(reinterpret_cast<byte*>(out_it), 6); // "Hello " 6 bytes
    int read_bytes2 = in_.Read(reinterpret_cast<byte*>(out_it) + 6, 15); // "123456\0" 7 bytes

    //Assert
    ASSERT_EQ(0, in_.GetCount());
    ASSERT_EQ(6, read_bytes1);
    ASSERT_EQ(7, read_bytes2);
    ASSERT_STREQ(expected_str, &*data.cbegin());
}

TEST_F(InPacketStreamFixture, When_count_is_less_than_sequence_Then_it_gets_count_bytes)
{
    //Arrange
    std::vector<char> data;
    in_.SetCount(6);

    //Act
    std::generate_n(std::back_inserter(data), 6, [this]{return in_.Get();});
    data.push_back('\0');
    int last_byte = in_.Get();

    //Assert
    ASSERT_TRUE(in_.IsEOF());
    ASSERT_EQ(0, in_.GetCount());
    ASSERT_EQ(-1, last_byte);
    ASSERT_STREQ("Hello ", &*data.cbegin());
}

TEST_F(InPacketStreamFixture, When_two_streams_in_pipe_Then_they_stream_both_datasets)
{
    //Arrange
    byte buffer[8] = {};
    Botan::SecureVector<byte> second_buffer;
    const char *sequence1 = "mark-x ";
    int len = strlen(sequence1);

    InPacketStreamMemory in1(reinterpret_cast<const byte*>(sequence1), reinterpret_cast<const byte*>(sequence1) + len);
    InPacketStreamPipe pipe;
    pipe.Push(in1);
    pipe.Push(in_);

    //Assert
    ASSERT_EQ(7 + 13, pipe.GetCount()); // "mark-x Hello 123456\0" 7 + 13 = 20 bytes

    //Act
    buffer[0] = pipe.Get();

    //Assert
    ASSERT_EQ('m', buffer[0]);
    ASSERT_EQ(19, pipe.GetCount());

    //Act
    pipe.Read(buffer, 2); //"ar"

    //Assert
    ASSERT_EQ('a', buffer[0]);
    ASSERT_EQ('r', buffer[1]);
    ASSERT_EQ(17, pipe.GetCount());

    //Arrange
    buffer[7] = '\0';

    //Act
    pipe.Read(buffer, 7); //"k-x Hel"

    //Assert
    ASSERT_STREQ("k-x Hel", reinterpret_cast<char*>(buffer));
    ASSERT_EQ(10, pipe.GetCount());

    //Arrange
    buffer[3] = '\0';

    //Act
    buffer[0] = pipe.Get();
    buffer[1] = pipe.Get();
    buffer[2] = pipe.Get();

    //Assert
    ASSERT_STREQ("lo ", reinterpret_cast<char*>(buffer));
    ASSERT_EQ(7, pipe.GetCount());
    ASSERT_FALSE(pipe.IsEOF());

    //Act
    int read_bytes = pipe.Read(buffer, 8);

    //Assert
    ASSERT_EQ(7, read_bytes);
    ASSERT_STREQ("123456", reinterpret_cast<char*>(buffer));
    ASSERT_EQ(0, pipe.GetCount());
    ASSERT_TRUE(pipe.IsEOF());
}

TEST_F(InPacketStreamFixture, When_writing_to_OutPacketStreamCont_It_populates_an_empty_container)
{
    //Arrange
    const char *sequence = "Hello Array 1234567";
    size_t len = strlen(sequence);
    
    Botan::SecureVector<byte> buffer;
    OutPacketStreamCont out(buffer);

    //Act
    out.Put('>');
    out.Write(reinterpret_cast<const byte*>(sequence), len);
    out.Put('<');
    out.Put(0);

    //Assert
    ASSERT_EQ(22, out.GetCount());
    ASSERT_STREQ(">Hello Array 1234567<", reinterpret_cast<const char*>(buffer.data()));
}

TEST_F(InPacketStreamFixture, When_writing_to_OutPacketStreamCont_It_populates_a_half_full_container)
{
    //Arrange
    const char *sequence = "Hello Array 1234567";
    size_t len = strlen(sequence);
    
    Botan::SecureVector<byte> buffer;
    buffer.resize(10);
    OutPacketStreamCont out(buffer);

    //Act
    out.Write(reinterpret_cast<const byte*>(sequence), len + 1);

    //Assert
    ASSERT_EQ(20, out.GetCount());
    ASSERT_GE(buffer.size(), 20U);
    ASSERT_STREQ("Hello Array 1234567", reinterpret_cast<const char*>(buffer.data()));
}

TEST_F(InPacketStreamFixture, When_writing_to_OutPacketStreamCont_incrementally_It_populates_an_empty_container)
{
    //Arrange
    std::string str = "Hello Array 1234567";
    
    Botan::SecureVector<byte> buffer;
    OutPacketStreamCont out(buffer);

    //Act
    auto it = str.begin();
    for(;it != str.end() - 10; it++)
    {
        out.Write(reinterpret_cast<byte*>(&(*it)), 1);
    }

    for(;it != str.end(); it++)
    {
        out.Put(*it);
    }
    out.Put(0);

    //Assert
    ASSERT_EQ(20, out.GetCount());
    ASSERT_GE(buffer.size(), 20);
    ASSERT_STREQ("Hello Array 1234567", reinterpret_cast<const char*>(buffer.data()));
}
// end of PacketStream tests

