#pragma once
#include "encryptmsg/algo_spec.h"

namespace EncryptPad
{
    const EncryptMsg::HashAlgo kDefaultHashAlgo = EncryptMsg::HashAlgo::SHA256;
    const EncryptMsg::CipherAlgo kDefaultCipherAlgo = EncryptMsg::CipherAlgo::AES256;
    const int kDefaultIterations = 1015808;
    const int kMaxIterations = 65011712;
    const int kDefaultKeyFileKeyLength = 512 >> 3;
    // TODO: restore the production buffer size
    // const size_t kDefaultMemoryBuffer = 10 << 20; // 10 Mb 
    const size_t kDefaultMemoryBuffer = 65536;
    const EncryptMsg::Compression kDefaultCompression = EncryptMsg::Compression::ZLIB; 
}

