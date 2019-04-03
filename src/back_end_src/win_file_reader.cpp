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
#if defined(__MINGW__) || defined(__MINGW32__)

#include "win_file_reader.h"
#include <fstream>

#include "wchar.h"
#include "stringapiset.h"
#include "windows.h"
#include "userenv.h"
#include "io.h"
#include "fcntl.h"

namespace
{
    void Multi2Wide(const std::string &multi, std::wstring &wide)
    {
        wide.clear();
        int len = MultiByteToWideChar(CP_UTF8, 0, multi.c_str(), -1,
                nullptr, 0);
        if(!len)
            return;

        wide.resize(len - 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, multi.data(), multi.size(),
                &wide[0], wide.size());
    }

    void Wide2Multi(const std::wstring &wide, std::string &multi)
    {
        multi.clear();

        if(wide.empty()) 
            return;

        int len = WideCharToMultiByte(CP_UTF8, 0, &wide[0], static_cast<int>(wide.size()),
                nullptr, 0, nullptr, nullptr);

        multi.resize(len - 1, 0);

        WideCharToMultiByte(CP_UTF8, 0, &wide[0], static_cast<int>(wide.size()),
                &multi[0], len, nullptr, nullptr);
    }
}

namespace EncryptPad
{
    std::string ExpandVariablesWin(const std::string &path)
    {
        std::wstring wide;
        Multi2Wide(path, wide);

        std::wstring buffer;
        buffer.resize(MAX_PATH);
        DWORD size = ExpandEnvironmentStringsW(wide.c_str(), &buffer[0], buffer.size());
        while(size == buffer.size())
        {
            buffer.resize(buffer.size() * 2);
            size = ExpandEnvironmentStringsW(wide.c_str(), &buffer[0], buffer.size());
        }
        buffer.resize(size);
        wide = buffer;
        std::string multi;
        Wide2Multi(wide, multi);
        return multi;
    }

    FileHndl OpenInputWin(const std::string &file_name)
    {
        if(file_name == "-")
        {
            if(_setmode(fileno(stdin), _O_BINARY) == -1)
                return FileHndl();

            return FileHndl(stdin);
        }

        std::wstring wide;
        Multi2Wide(file_name, wide);
        return FileHndl(_wfopen(wide.data(), L"rb"));
    }

    FileHndl OpenOutputWin(const std::string &file_name)
    {
        if(file_name == "-")
        {
            if(_setmode(fileno(stdout), _O_BINARY) == -1)
                return FileHndl();

            return FileHndl(stdout);
        }

        std::wstring wide;
        Multi2Wide(file_name, wide);
        return FileHndl(_wfopen(wide.data(), L"wb"));
    }

    bool RemoveFileWin(const std::string &file_name)
    {
        std::wstring wide;
        Multi2Wide(file_name, wide);
        return _wremove(wide.data()) == 0;
    }

    stream_length_type WinFTell(FileHndl &file)
    {
		return _ftelli64(file.get());
    }

    int WinFSeek(FileHndl &file, stream_length_type offset, int origin)
    {
		return _fseeki64(file.get(), offset, origin);
    }
}
#endif
