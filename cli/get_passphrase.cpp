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
#include "get_passphrase.h"
#include <string>
#include <iostream>
#include "assert.h"
#include "epad_utilities.h"

#if defined(__MINGW32__) || defined(_MSC_VER)

#include <windows.h>

namespace
{
    class HandleTraits
    {
    public:
        static HANDLE InvalidHandle()
        {
            return INVALID_HANDLE_VALUE;
        }

        static void Close(HANDLE h)
        {
            CloseHandle(h);
        }
    };

    typedef EncryptPad::UniqueHandler<HANDLE, HandleTraits> Hndl; 

    void WriteToConsole(const std::string &prompt)
    {
        Hndl h = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if(!h.Valid())
        {
            std::cerr << "Cannot open the terminal for output" << std::endl;
            return;
        }

        FileHndl file = fopen("CONOUT$", "w", "t");
        if(!file.Valid())
        {
            std::cerr << "Cannot open the terminal descriptor for output" << std::endl;
            return;
        }

        fputs(prompt.data(), file.get());
    }
}

namespace EncryptPad
{
    void GetPassphrase(const std::string &prompt, std::string &passphrase)
    {
        const char kBackSpace=8;
        const char kReturn=13;

        passphrase.clear();

        unsigned char ch=0;

        WriteToConsole(prompt);

        DWORD con_mode;
        DWORD read_word;
        Hndl h = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if(!h.Valid())
        {
            std::cerr << "Cannot open the terminal for input" << std::endl;
            return;
        }

        GetConsoleMode(h.get(), &con_mode);
        SetConsoleMode(h.get(), con_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));

        FileHndl file=fopen("CONIN$", "r", "t");
        if(!file.Valid())
        {
            std::cerr << "Cannot open the terminal descriptor for input" << std::endl;
            return;
        }

        int read_result = -1;
        while((read_result = fgetc(file.get()))!=EOF && (ch = static_cast<unsigned char>(read_result)) != kReturn)
        {
            if(ch == kBackSpace)
            {
                if(passphrase.length() != 0)
                {
                    passphrase.resize(passphrase.length() - 1);
                }
            }
            else
            {
                passphrase+=ch;
            }
        }

        WriteToConsole("\n");
    }

}

#elif defined(__APPLE__) || defined(unix) || defined(__unix__) || defined(__unix)

#include "termios.h"
#include "unistd.h"
#include <cstdio>

namespace
{
    const char kBackspace = 127;
    const char kReturn = 10;
    const char kInvalid = -1;

    char Getch(FILE *f)
    {
        struct termios t_old, t_new;
        int file_no = fileno(f);

        tcgetattr(file_no, &t_old);
        t_new = t_old;
        t_new.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(file_no, TCSANOW, &t_new);

        char ch;
        if(fread(&ch, 1, 1, f) != 1)
        {
            ch = kInvalid;
        }

        tcsetattr(file_no, TCSANOW, &t_old);
        return ch;
    }
}

namespace EncryptPad
{
    void GetPassphrase(const std::string &prompt, std::string &passphrase)
    {
        passphrase.clear();
        unsigned char ch=0;

        const char *termid = ctermid(NULL);
        FileHndl term_file(fopen(termid, "r+"));
        if(!term_file.Valid())
        {
            std::cerr << "Cannot open the terminal for output" << std::endl;
            return;
        }

        size_t res = fwrite(prompt.c_str(), 1, prompt.size(), term_file.get());
        assert(res == prompt.size());
        (void)res;

        while((ch = Getch(term_file.get())) != kReturn && ch != kInvalid)
        {
            if(ch == kBackspace)
            {
                if(passphrase.length() != 0)
                {
                    passphrase.resize(passphrase.length() - 1);
                }
            }
            else
            {
                passphrase += ch;
            }
        }

        fputc('\n', term_file.get());
    }
}
#endif
