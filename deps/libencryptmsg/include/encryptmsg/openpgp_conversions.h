#pragma once
#include <cstdint>
#include <cstddef>

namespace LibEncryptMsg
{
    uint8_t EncodeS2KIterations (size_t iterations);
    size_t DecodeS2KIterations(uint8_t count);
}

