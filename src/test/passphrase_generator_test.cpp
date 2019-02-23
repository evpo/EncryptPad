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

#include "gtest/gtest.h"
#include "passphrase_generator.h"
#include <algorithm>

using namespace EncryptPad;
using namespace std;

vector<CharSet> GetCharSets()
{
    return GetCharSets(CharRange::All);
}

TEST(PassphraseGenerator, It_generates_correct_number_of_passphrases)
{
    //Arrange
    int pwd_count = 5;

    //Act
    vector<string> passphrases = GeneratePassphrases(GetCharSets(), 12, pwd_count);

    //Assert
    ASSERT_EQ(5U, passphrases.size());

}

TEST(PassphraseGenerator, When_no_char_sets_It_returns_empty_collection)
{
    //Arrange
    vector<CharSet> sets;

    //Act
    vector<string> passphrases = GeneratePassphrases(sets, 8, 3);

    //Assert
    ASSERT_TRUE(passphrases.empty());
}

TEST(PassphraseGenerator, When_max_char_count_is_less_than_passphrase_length_Generated_passphrases_are_cut_to_max_char_count)
{
    //Arrange
    int pwd_length = 12;
    vector<CharSet> sets;
    sets.push_back(CharSet(GetLowerAtoZ(), CharRange::LowerAtoZ, 1, 2));
    sets.push_back(CharSet(GetUpperAtoZ(), CharRange::UpperAtoZ, 1, 3));

    //Act
    vector<string> passphrases = GeneratePassphrases(sets, pwd_length, 3);

    //Assert
    ASSERT_EQ(3U, passphrases.size());
    ASSERT_EQ(5U, passphrases[0].size());
    ASSERT_EQ(5U, passphrases[1].size());
    ASSERT_EQ(5U, passphrases[2].size());
}

TEST(PassphraseGenerator, Boundary_case_in_which_max_char_count_is_passphrase_length)
{
    //Arrange
    unsigned int pwd_length = 12;
    vector<CharSet> sets;
    sets.push_back(CharSet(GetLowerAtoZ(), CharRange::LowerAtoZ, 1, pwd_length));

    //Act
    vector<string> passphrases = GeneratePassphrases(sets, pwd_length, 3);

    //Assert
    ASSERT_EQ(3U, passphrases.size());
    ASSERT_EQ(pwd_length, passphrases[0].size());
    ASSERT_EQ(pwd_length, passphrases[1].size());
    ASSERT_EQ(pwd_length, passphrases[2].size());
}

TEST(PassphraseGenerator, It_generates_passphrases_of_correct_length)
{
    //Arrange
    unsigned int pwd_length = 12;

    //Act
    vector<string> passphrases = GeneratePassphrases(GetCharSets(), pwd_length, 3);

    //Assert
    ASSERT_EQ(3U, passphrases.size());
    ASSERT_EQ(pwd_length, passphrases[0].size());
    ASSERT_EQ(pwd_length, passphrases[1].size());
    ASSERT_EQ(pwd_length, passphrases[2].size());
}

TEST(PassphraseGenerator, Boundary_test_in_which_min_char_count_takes_all_chars)
{
    //Arrange
    vector<CharSet> sets;
    sets.push_back(CharSet(GetLowerAtoZ(), CharRange::LowerAtoZ, 1, 1));
    sets.push_back(CharSet(GetUpperAtoZ(), CharRange::UpperAtoZ, 1, 1));

    //Act
    vector<string> passphrases = GeneratePassphrases(sets, 2, 1);

    //Assert
    ASSERT_EQ(1U, passphrases.size());
    string &pwd = passphrases[0];
    ASSERT_EQ(2U, pwd.size());
    bool lower1 = GetLowerAtoZ().find(pwd[0]) != string::npos; 
    bool lower2 = GetLowerAtoZ().find(pwd[1]) != string::npos; 
    bool upper1 = GetUpperAtoZ().find(pwd[0]) != string::npos; 
    bool upper2 = GetUpperAtoZ().find(pwd[1]) != string::npos; 
    ASSERT_TRUE((lower1 && upper2) || (upper1 && lower2));
}

TEST(PassphraseGenerator, Boundary_test_in_which_max_char_count_is_one)
{
    //Arrange
    vector<CharSet> sets;
    sets.push_back(CharSet(GetLowerAtoZ()));
    const string &upper_chars = GetUpperAtoZ();
    unsigned max_char_count = 1;
    sets.push_back(CharSet(GetUpperAtoZ(), CharRange::UpperAtoZ, 1, max_char_count));

    //Act
    vector<string> passphrases = GeneratePassphrases(sets, 100, 1);

    //Assert
    ASSERT_EQ(1U, passphrases.size());
    string &pwd = passphrases[0];

    ASSERT_EQ(100U, pwd.size());

    std::sort(pwd.begin(), pwd.end());

    string buf;
    buf.resize(upper_chars.size());
    auto out_it = std::set_intersection(
            pwd.begin(), pwd.end(), upper_chars.begin(), upper_chars.end(), buf.begin());

    ASSERT_EQ(1, out_it - buf.begin());
}

TEST(PassphraseGenerator, Passphrases_do_not_exceed_char_set_maximum_and_reach_char_set_minimum)
{
    //Arrange
    vector<CharSet> sets;
    sets.push_back(CharSet(GetLowerAtoZ()));
    sets.push_back(CharSet(GetUpperAtoZ(), CharRange::UpperAtoZ, 1, 3));

    //Act
    vector<string> passphrases = GeneratePassphrases(sets, 100, 1000);
    
    //Assert
    ASSERT_EQ(1000U, passphrases.size());

    const string &upperChars = GetUpperAtoZ();
    string buf;
    buf.resize(upperChars.size());

    auto pwd_it = passphrases.begin();
    for(; pwd_it != passphrases.end(); pwd_it ++)
    {
        ASSERT_EQ(100U, pwd_it->size());
        std::sort(pwd_it->begin(), pwd_it->end());
        auto out_it = std::set_intersection(
                pwd_it->begin(), pwd_it->end(), upperChars.begin(), upperChars.end(), buf.begin());
        ASSERT_GE(3, out_it - buf.begin());
        ASSERT_LE(1, out_it - buf.begin());
    }
}

TEST(PassphraseGenerator, Passphrases_contain_all_sets)
{
    //Arrange
    auto sets = GetCharSets();

    //Act
    vector<string> passphrases = GeneratePassphrases(sets, 6, 1000);

    //Assert
    vector<string>::iterator pwd_it = passphrases.begin();
    vector<char> buf;

    ASSERT_EQ(1000U, passphrases.size());

    for(;pwd_it != passphrases.end(); pwd_it++)
    {
        string &pwd = *pwd_it;
        std::sort(pwd.begin(), pwd.end());
        auto set_it = sets.begin();
        for(;set_it != sets.end(); set_it ++)
        {
            std::sort(set_it->chars.begin(), set_it->chars.end());
            buf.resize(set_it->chars.size());
            auto out_it = std::set_intersection(
                    set_it->chars.begin(), set_it->chars.end(), pwd_it->begin(), pwd_it->end(), buf.begin());
            ASSERT_NE(buf.begin(), out_it);
        }
    }
}
