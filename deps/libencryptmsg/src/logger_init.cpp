#include "logger_init.h"
#include "plog/Log.h"

namespace LibEncryptMsg
{
    void InitLogger(const std::string &file_name)
    {
        plog::init(plog::debug, file_name.data());
        LOG_INFO << "Log instance started";
    }
}
