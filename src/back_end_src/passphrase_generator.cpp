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
#include "passphrase_generator.h"
#include <cstring>
#include <stack>
#include "assert.h"
#include "botan.h"
#include "botan/auto_rng.h"

using namespace std;

namespace 
{
    unsigned int GetRandomNumber(Botan::AutoSeeded_RNG &rng, unsigned min, unsigned max)
    {
        assert(max >= min);
        assert(max < 0xFFFF);
        static_assert(sizeof(unsigned short) == 2, "unsigned short is not 2 bytes");

        if(max == min)
            return min;

        unsigned val = rng.next_byte();
        val <<= 8;
        val += rng.next_byte();

        val = (val % (max - min + 1)) + min;
        return val;
    }
}

namespace EncryptPad
{
    vector<string> GeneratePassphrases(const vector<CharSet> &char_sets, int pwd_length, int pwd_count)
    {
        assert(pwd_length > 0);
        assert(pwd_count > 0);

        if(char_sets.empty())
        {
            return vector<string>();
        }

        vector<CharSet> sets = char_sets;
        unsigned org_total_chars = 0;
        auto set_it = sets.begin();
        for(;set_it != sets.end(); set_it++)
        {
            assert(set_it->chars.size() >= 1);
            org_total_chars += set_it->chars.size();
        }

        vector<string> ret_val;
		Botan::AutoSeeded_RNG rng;

        while(pwd_count--)
        {
            string pwd;
            pwd.resize(pwd_length);
            auto char_it = pwd.begin();

            sets = char_sets;
            unsigned total_chars = org_total_chars;
            stack<unsigned> maxed_out_sets;

            // Add required minimum
            set_it = sets.begin();
            for(; set_it != sets.end() && char_it != pwd.end(); set_it ++)
            {
                assert(set_it->max >= set_it->min);

                for(;set_it->min > 0 && char_it != pwd.end(); char_it ++)
                {
                    *char_it = set_it->chars.at(GetRandomNumber(rng, 0, set_it->chars.size() - 1));
                    set_it->min --;
                    set_it->max --;
                    if(set_it->max == 0)
                        maxed_out_sets.push(set_it - sets.begin());
                }
            }

            // Remove character sets reaching characters limits
            while(!maxed_out_sets.empty())
            {
                unsigned i = maxed_out_sets.top();
                maxed_out_sets.pop();
                set_it = sets.begin() + i;
                assert(set_it->max == 0);
                total_chars -= set_it->chars.size();
                sets.erase(set_it);
            }

            assert(char_it <= pwd.end());

            for(;char_it != pwd.end() && !sets.empty(); char_it++)
            {
                assert(total_chars >= 1);
                unsigned char_index = GetRandomNumber(rng, 0, total_chars - 1);
                set_it = sets.begin();

                for(; set_it != sets.end() && char_index >= set_it->chars.size(); set_it++)
                {
                    assert(set_it->chars.size() <= char_index);
                    char_index -= set_it->chars.size();
                }

                assert(set_it != sets.end());

                *char_it = set_it->chars.at(char_index);
                set_it->max --;
                if(set_it->max == 0)
                {
                    total_chars -= set_it->chars.size();
                    sets.erase(set_it);
                }
            }

            pwd.resize(char_it - pwd.begin());

            char_it = pwd.begin();
            for(;char_it != pwd.end(); char_it++)
            {
                assert(pwd.end() > char_it);
                unsigned index = GetRandomNumber(rng, 0, pwd.end() - char_it - 1);
                assert(char_it + index < pwd.end());
                std::swap(*char_it, *(char_it + index)); 
            }

            ret_val.push_back(pwd);
        }

        return ret_val;
    }

    void AddRangeIfEmpty(string &buf, char start, char end)
    {
        if(!buf.empty())
            return;

        int size = end + 1 - start;
        buf.resize(size);
        char c = start;
        for(int i = 0; i < size; i++, c++)
        {
            buf[i] = c;
        }
    }

    const std::string &GetLowerAtoZ()
    {
        static string buf;
        AddRangeIfEmpty(buf, 'a', 'z');
        return buf;
    }

    const std::string &GetUpperAtoZ()
    {
        static string buf;
        AddRangeIfEmpty(buf, 'A', 'Z');
        return buf;
    }

    const std::string &GetNumbers()
    {
        static string buf;
        AddRangeIfEmpty(buf, '0', '9');
        return buf;
    }

    const std::string &GetSymbols()
    {
        static string buf;
        if(buf.empty())
        {
            char symbols[] = "`~!@#$%^&*()_-+={}[]\\|:;\"'<>,.?/";
            int length = char_traits<char>::length(symbols);
            buf = string(symbols, symbols + length);
        }
        return buf;
    }

    vector<CharSet> GetCharSets(CharRange ranges)
    {
        vector<CharSet> ret_val;

        for(int i = 1; i & static_cast<int>(CharRange::All); i <<= 1)
        {
            if(! (i & static_cast<int>(ranges)))
                continue;

            CharRange range = static_cast<CharRange>(i);
            switch(range)
            {
                case CharRange::LowerAtoZ:
                    ret_val.push_back(CharSet(GetLowerAtoZ(), range));
                    break;
                case CharRange::UpperAtoZ:
                    ret_val.push_back(CharSet(GetUpperAtoZ(), range));
                    break;
                case CharRange::Numbers:
                    ret_val.push_back(CharSet(GetNumbers(), range));
                    break;
                case CharRange::Symbols:
                    ret_val.push_back(CharSet(GetSymbols(), range));
                    break;
                case CharRange::Other:
                    // ignore
                    break;
                default:
                    assert(false);
                    return ret_val;
            }
        }

        return ret_val;
    }
}
