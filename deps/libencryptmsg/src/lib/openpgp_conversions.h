//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <cstdint>
#include <cstddef>
#include <stdexcept>

namespace EncryptMsg
{
    uint8_t EncodeS2KIterations (size_t iterations);
    size_t DecodeS2KIterations(uint8_t count);
}

