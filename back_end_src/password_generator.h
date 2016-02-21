//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
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
#pragma once
#include <string>
#include <vector>

namespace EncryptPad
{
    enum class CharRange
    {
        None = 0x00,
        LowerAtoZ = 0x01,
        UpperAtoZ = 0x02,
        Numbers = 0x04,
        Symbols = 0x08,
        All = 0x0F,
        Other = 0x8000,
    };

    inline CharRange operator|(CharRange a, CharRange b)
    {
        return static_cast<CharRange>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline CharRange operator&(CharRange a, CharRange b)
    {
        return static_cast<CharRange>(static_cast<int>(a) & static_cast<int>(b));
    }

    struct CharSet
    {
        std::string chars;
        unsigned min;
        unsigned max;
        CharRange range;

        CharSet():min(1), max(0xFF), range(CharRange::Other){}
        CharSet(const std::string &chars_p, CharRange range = CharRange::Other, int min_p = 1, int max_p = 0xFF)
            :chars(chars_p), min(min_p), max(max_p), range(range){}
    };

    std::vector<std::string> GeneratePasswords(const std::vector<CharSet> &char_sets, int pwd_length, int pwd_count);

    const std::string &GetLowerAtoZ();
    const std::string &GetUpperAtoZ();
    const std::string &GetNumbers();
    const std::string &GetSymbols();

    std::vector<CharSet> GetCharSets(CharRange ranges);
}

