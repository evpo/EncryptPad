#pragma once
#include "emsg_types.h"

namespace EncryptPad
{
    enum class EpadResult
    {
        //These correspond to EmsgResult (see ToEpadResult)
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
        //End of block

        Empty,
        IOError,
        IOErrorInput,
        IOErrorOutput,
        CompressionError,
        KeyFileNotSpecified,
        IOErrorKeyFile,
        InvalidKeyFile,
        CurlIsNotFound,
        CurlExitNonZero,
        InvalidWadFile,
        InvalidOrIncompleteWadFile,
        KeyIsRequiredForSaving,
        InvalidKeyFilePassphrase,
        Cancelled,
    };

    inline EpadResult ToEpadResult(LibEncryptMsg::EmsgResult emsg_result)
    {
        return static_cast<EpadResult>(emsg_result);
    }
}

