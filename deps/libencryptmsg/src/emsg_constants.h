#pragma once
#include <cstddef>
#include <limits>
#include "algo_spec.h"

namespace LibEncryptMsg
{
    const size_t kMaxPacketChainLength = 5;
    const size_t kSaltSize = 8;
    const size_t kMaxStreamLength = std::numeric_limits<size_t>::max();

    const HashAlgo kDefaultHashAlgo = HashAlgo::SHA256;
    const CipherAlgo kDefaultCipherAlgo = CipherAlgo::AES256;
    const int kDefaultIterations = 1015808;
    const int kMaxIterations = 65011712;
    const Compression kDefaultCompression = Compression::ZLIB;
    const uint8_t kDefaultPartialLengthPower = 4;
    const bool kDefaultBinary = true;
}

