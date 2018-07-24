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
#include "x2_key_loader.h"
#include <fstream>
#include "file_system.hpp"
#include "curl_loader.h"
#include "os_api.h"
#include "packet_typedef.h"
#include "epad_utilities.h"
#include "repository.h"

using namespace std;
using namespace Botan;
using namespace EncryptMsg;

namespace EncryptPad
{
    bool IsUrl(const string& path)
    {
        auto pos = path.find(':');
        if(pos == string::npos)
            return false;
        if(pos < 2) // could be a drive
            return false;

        if(pos > path.length() - 3) // pr://
            return false;

        if(find_if(path.begin(), path.begin() + pos, [](char c){return isalpha((int)c) == 0;}) != path.begin() + pos)
            return false;

        if(path[pos + 1] != '/' && path[pos + 2] != '/')
            return false;

        return true;
    }

    EpadResult LoadKeyFromFile(const string& file_name, const std::string &libcurl_path, const std::string &libcurl_params, std::string &key)
    {
        if(!libcurl_path.empty() && IsUrl(file_name))
            return LoadKeyFromFileThroughCurl(file_name, libcurl_path, libcurl_params, key);

        std::string path = ExpandVariables(file_name);

        if (LoadStringFromFile(path, key))
            return EpadResult::Success;

        // It does not contain directories
        if (stlplus::folder_part(path).empty())
        {
            std::string keyStoragePath = stlplus::create_filespec(GetRepositoryPath(), path);
            if(LoadStringFromFile(keyStoragePath, key))
               return EpadResult::Success;
        }

        return EpadResult::IOErrorKeyFile;
    }
}
