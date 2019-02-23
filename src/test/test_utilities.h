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
#include <cstdio>

namespace EncryptPad
{
    class TestCleanUp
    {
    private:
        std::string file2clean_;
    public:
        TestCleanUp(const std::string &file_name):file2clean_(file_name)
        {
        }

        ~TestCleanUp()
        {
            remove(file2clean_.c_str());
        }
    };
}
