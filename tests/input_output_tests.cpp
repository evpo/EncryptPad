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
#include <fstream>
#include <cstdio>
#include <vector>
#include "gtest/gtest.h"
#include "botan.h"
#include "test_utilities.h"
#include "file_helper.h"

using namespace EncryptPad;

TEST(InputOutput, When_buffer_loaded_from_descriptor_Buffer_matches_file_content)
{
    // Arrange
    std::string file_name = "temp1.txt";
    std::string content = "one two three\nfour five six\n";

    EncryptPad::TestCleanUp cleanup(file_name);

    std::ofstream stm(file_name, std::ios_base::out);
    stm.write(content.data(), content.size());
    stm.close();

    FileHndl file = fopen(file_name.c_str(), "r");
    int fd = fileno(file.get());

    std::vector<byte> buffer;

    //Act
    bool result = LoadFromIOStream(fd, buffer);

    //Assert
    ASSERT_TRUE(result);
    ASSERT_STREQ(content.c_str(), (const char*)buffer.data());
}


TEST(InputOutput, When_buffer_saved_to_descriptor_File_matches_buffer_content)
{
    // Arrange
    std::string content = "one two three\nfour five six\n";
    Botan::SecureVector<byte> buffer;
    buffer.resize(content.size());
    std::copy_n(reinterpret_cast<const byte*>(content.data()), content.size(), buffer.begin());

    std::string file_name = "temp1.txt";
    EncryptPad::TestCleanUp cleanup(file_name);
    bool result = false;

    {
        FileHndl file = fopen(file_name.c_str(), "w");
        int fd = fileno(file.get());

        //Act
        result = SaveToIOStream(fd, buffer);
    }

    //Assert
    std::ifstream stm(file_name, std::ios_base::in);
    std::string from_file;
    from_file.resize(content.size());
    stm.read(&from_file[0], from_file.size());
    stm.get();

    ASSERT_TRUE(result);
    ASSERT_TRUE(stm.eof());
    ASSERT_STREQ(content.c_str(), from_file.c_str());
}
