#pragma once
#include <string>

namespace EncryptPad
{
    void InitializeRepositoryPath(const std::string &arg0);
    // return empty string if the folder does not exist and cannot be created
    const std::string &GetRepositoryPath();
}

