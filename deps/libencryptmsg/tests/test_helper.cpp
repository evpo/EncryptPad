#include "test_helper.h"


namespace EncryptMsg
{
    namespace UnitTests
    {
        const uint8_t *FromChar(const char *data)
        {
            return reinterpret_cast<const uint8_t *>(data);
        }
    }
}
