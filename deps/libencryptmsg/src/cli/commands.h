#pragma once
#include <string>

namespace EncryptMsg
{
    namespace Cli
    {
        void Encrypt(const std::string &input, const std::string &output, const std::string &pwd_file);
        void Decrypt(const std::string &input, const std::string &output, const std::string &pwd_file);
    }
}
