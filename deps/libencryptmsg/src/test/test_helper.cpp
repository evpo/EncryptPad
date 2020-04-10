#include "test_helper.h"
#include <fstream>


namespace EncryptMsg
{
    namespace UnitTests
    {
        const uint8_t *FromChar(const char *data)
        {
            return reinterpret_cast<const uint8_t *>(data);
        }

        void LoadFile(const std::string &file_name, std::vector<uint8_t> &buf, bool binary)
        {
            auto open_mode = binary ? (std::ios_base::in | std::ios_base::binary)
                : std::ios_base::in;
            std::ifstream stm(file_name, open_mode);
            std::copy(std::istreambuf_iterator<char>(stm), std::istreambuf_iterator<char>(),
                    std::back_inserter(buf));
        }
    }
}
