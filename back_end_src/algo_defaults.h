#pragma once
#include "algo_spec.h"

namespace EncryptPad
{
    const LibEncryptMsg::HashAlgo kDefaultHashAlgo = LibEncryptMsg::HashAlgo::SHA256;
    const LibEncryptMsg::CipherAlgo kDefaultCipherAlgo = LibEncryptMsg::CipherAlgo::AES256;
    const int kDefaultIterations = 1015808;
    const int kMaxIterations = 65011712;
    const int kDefaultKeyFileKeyLength = 512 >> 3;
    // TODO: restore the production buffer size
    // const size_t kDefaultMemoryBuffer = 768 << 20; // 768 Mb 
    const size_t kDefaultMemoryBuffer = 10 << 20; // 10 Mb 
    const LibEncryptMsg::Compression kDefaultCompression = LibEncryptMsg::Compression::ZLIB; 
}

