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
#include "curl_loader.h"
#include "subprocesses.hpp"

using namespace stlplus;
using namespace LibEncryptMsg;

namespace {
    class CurlSubprocess : public subprocess
    {
    public:
        const std::string &get_output()
        {
            return output;
        }
    protected:
        std::string output;
        virtual bool callback()
        {
            while(read_stdout(output)>=0);
            return true;
        }
    };
}

namespace EncryptPad
{
    EpadResult LoadKeyFromFileThroughCurl(const std::string& file_name, const std::string &libcurl_path,
            const std::string &libcurl_params, std::string &key)
    {
        arg_vector arg_v(libcurl_path + " " + libcurl_params + " " + file_name);
        CurlSubprocess sub_proc;
        bool result = sub_proc.spawn(libcurl_path, arg_v, false, true, false);
        if(!result)
        {
            switch(sub_proc.error_number())
            {
                case 2:
                case 3:
                    return EpadResult::CurlIsNotFound;
                default:
                    return EpadResult::IOErrorKeyFile;
            }
        }

        if(sub_proc.get_output().length() == 0)
            return EpadResult::IOErrorKeyFile;
        if(sub_proc.exit_status() != 0)
            return EpadResult::CurlExitNonZero;

        key = sub_proc.get_output();
        // key = Botan::OctetString(Botan::base64_decode(sub_proc.get_output()));
        return EpadResult::Success;
    }
}
