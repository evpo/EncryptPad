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
#include <string>
#include <fstream>
#include "gtest/gtest.h"
#include "key_generation.h"
#include "test_utilities.h"
#include "encryptmsg/algo_spec.h"
#include "algo_defaults.h"

using namespace EncryptPad;

TEST(KeyGeneration, Random_bytes_are_generated)
{
    const size_t buffer_length = 32;
    unsigned char buffer[buffer_length];

    GenerateNewKey(buffer, buffer_length);
    std::string str(reinterpret_cast<const char*>(buffer), buffer_length);

    GenerateNewKey(buffer, buffer_length);
    std::string str_next(reinterpret_cast<const char*>(buffer), buffer_length);

    ASSERT_NE(str, str_next);
}

TEST(KeyGeneration, Key_file_is_created_and_it_is_unique)
{
    std::string tmp_file1 = "tmp_file1.key", tmp_file2 = "tmp_file2.key";
    TestCleanUp clean_up1(tmp_file1), clean_up2(tmp_file2);
    GenerateNewKey(tmp_file1, kDefaultKeyFileKeyLength);
    GenerateNewKey(tmp_file2, kDefaultKeyFileKeyLength);

    std::ifstream stm1(tmp_file1, std::ios_base::in);
    std::string key1;
    stm1 >> key1;

    std::ifstream stm2(tmp_file2, std::ios_base::in);
    std::string key2;
    stm2 >> key2;

    ASSERT_NE(key1, key2);
}
