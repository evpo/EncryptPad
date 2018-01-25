#include "encryptmsg_version.h"

#include <string>
#include "encryptmsg/version.h"

namespace EncryptPad
{
    std::string EncryptMsgVersion()
    {
        return VER_PRODUCTNAME_STR " " VER_PRODUCTVERSION_STR;
    }
}

