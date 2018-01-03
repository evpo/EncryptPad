#pragma once
#include <vector>
#include "botan/symkey.h"

namespace LibEncryptMsg
{
    enum class EmsgResult
    {
        Success = 0,
        Pending,
        InvalidSurrogateIV,
        UnexpectedFormat,
        UnsupportedPacketType,
        UnsupportedAlgo,
        UnsupportedS2K,
        UnsupportedCompressionAlgo,
        MDCError,
        InvalidPassphrase,
        None,
    };

    using FileDate = unsigned;
    using Salt = std::vector<uint8_t>;
    using EncryptionKey = Botan::SymmetricKey;
}
