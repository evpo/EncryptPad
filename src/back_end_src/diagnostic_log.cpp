#include "diagnostic_log.h"

namespace EncryptPad
{

plog::Severity ParsePlogSeverity(const std::string &str)
{
    using namespace plog;
    if(str == "none")
    {
        return Severity::none;
    }
    else if(str == "fatal")
    {
        return Severity::fatal;
    }
    else if(str == "error")
    {
        return Severity::error;
    }
    else if(str == "warning")
    {
        return Severity::warning;
    }
    else if(str == "info")
    {
        return Severity::info;
    }
    else if(str == "debug")
    {
        return Severity::debug;
    }
    else if(str == "verbose")
    {
        return Severity::verbose;
    }
    else
    {
        return Severity::warning;
    }
}

}
