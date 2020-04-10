#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace EncryptMsg
{
    namespace UnitTests
    {
        const uint8_t *FromChar(const char *);
        void LoadFile(const std::string &file_name, std::vector<uint8_t> &buf, bool binary = true);
    }
}
