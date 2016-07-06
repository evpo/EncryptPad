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
#include "os_api.h"

#if defined(__MINGW32__) || defined(_MSC_VER)
#include "win_file_reader.h"
#else
#include <sstream>
#include "wordexp.h"
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
	std::string ExpandVariables(const std::string &path)
	{
        return EncryptPad::ExpandVariablesWin(path);
	}
#endif

#if defined(__APPLE__) || defined(unix) || defined(__unix__) || defined(__unix)
	std::string ExpandVariables(const std::string &path)
    {
        wordexp_t p;
        int error_code = wordexp( path.c_str(), &p, 0 );
        if(error_code)
            return path;

        char **w;
        w = p.we_wordv;
        std::ostringstream stm;
        for (size_t i=0; i<p.we_wordc;i++ )
        {
            if(i > 0)
              stm << " ";
	    stm << w[i];
        }
        wordfree( &p );
        return stm.str();
    }
#endif

