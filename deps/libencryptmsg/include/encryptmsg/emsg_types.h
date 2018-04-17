//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <vector>
#include "botan/symkey.h"

namespace EncryptMsg
{
    enum class EmsgResult
    {
        Success = 0,
        None = 1,
        Pending = 2,
        InvalidSurrogateIV = 3,
        UnexpectedFormat = 4,
        UnsupportedPacketType = 5,
        UnsupportedAlgo = 6,
        UnsupportedS2K = 7,
        UnsupportedCompressionAlgo = 8,
        MDCError = 9,
        InvalidPassphrase = 10,
        UnexpectedError = 11,
    };

    using FileDate = unsigned;
    using Salt = std::vector<uint8_t>;
    using EncryptionKey = Botan::SymmetricKey;
}
