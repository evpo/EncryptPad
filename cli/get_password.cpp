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
#include "get_password.h"
#include <string>
#include <iostream>

#if defined(__MINGW32__) || defined(_MSC_VER)

#include <windows.h>

namespace EncryptPad
{
    void GetPassword(std::string &password)
    {
        const char kBackSpace=8;
        const char kReturn=13;

        password.clear();

        unsigned char ch=0;

        DWORD con_mode;
        DWORD read_word;

        HANDLE h = GetStdHandle(STD_INPUT_HANDLE);

        GetConsoleMode( h, &con_mode );
        SetConsoleMode( h, con_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT) );

        while(ReadConsoleA( h, &ch, 1, &read_word, NULL) && ch != kReturn)
        {
            if(ch == kBackSpace)
            {
                if(password.length() != 0)
                {
                    password.resize(password.length() - 1);
                }
            }
            else
            {
                password+=ch;
            }
        }
        std::cout << std::endl;
    }

}

#elif defined(__APPLE__) || defined(__linux__) 

#include <termios.h>
#include <unistd.h>
#include <stdio.h>

namespace
{
    int Getch() {
        int ch;
        struct termios t_old, t_new;

        tcgetattr(STDIN_FILENO, &t_old);
        t_new = t_old;
        t_new.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
        return ch;
    }
}

namespace EncryptPad
{
    void GetPassword(std::string &password)
    {
        const char kBackspace=127;
        const char kReturn=10;

        password.clear();
        unsigned char ch=0;

        while((ch = Getch()) != kReturn)
        {
            if(ch == kBackspace)
            {
                if(password.length() != 0)
                {
                    password.resize(password.length() - 1);
                }
            }
            else
            {
                password += ch;
            }
        }
        std::cout << std::endl;
    }
}
#endif
