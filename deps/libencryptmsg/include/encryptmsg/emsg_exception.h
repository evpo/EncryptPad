//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <string>
#include <exception>
#include "emsg_types.h"

namespace LibEncryptMsg
{
    class EmsgException : public std::exception
    {
        public:
            EmsgResult result;
            std::string message;
            const char *what() const noexcept override
            {
                return message.c_str();
            }

            EmsgException(EmsgResult result_p);
    };

    std::string InterpretEmsgResult(EmsgResult result);
}

