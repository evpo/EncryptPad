#pragma once
#include <string>
#include <exception>
#include "emsg_types.h"

namespace LibEncryptMsg
{
    class EmsgException : public std::exception
    {
        public:
            PacketResult result;
            std::string message;
            const char *what() const noexcept override
            {
                return message.c_str();
            }

            EmsgException(PacketResult result_p);
    };

}

