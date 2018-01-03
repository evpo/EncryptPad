#include "emsg_exception.h"
#include "assert.h"

namespace LibEncryptMsg
{
    std::string InterpretEmsgResult(EmsgResult result)
    {
        switch(result)
        {
            case EmsgResult::InvalidSurrogateIV:
                return "Incorrect passphrase";
            case EmsgResult::UnexpectedFormat:
                return "Unexpected format";
            case EmsgResult::UnsupportedPacketType:
                return "Packet type is not supported";
            case EmsgResult::UnsupportedAlgo:
                return "Encryption algorithm is not supported";
            case EmsgResult::UnsupportedS2K:
                return "S2K algorithm is not supported";
            case EmsgResult::UnsupportedCompressionAlgo:
                return "Unsupported compression algorithm";
            case EmsgResult::MDCError:
                return "File integrity check failed"; 
            case EmsgResult::InvalidPassphrase:
                return "Key service does not containt a suitable key";
            default:
                assert(false);
                return "";
        }
    }

    EmsgException::EmsgException(EmsgResult result_p):result(result_p)
    {
        message = InterpretEmsgResult(result);
    }
}

