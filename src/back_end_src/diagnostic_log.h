#pragma once
#include "plog/Severity.h"
#include <string>

namespace EncryptPad
{

plog::Severity ParsePlogSeverity(const std::string &str);

}


