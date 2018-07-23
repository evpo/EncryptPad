#include "repository.h"
#include "file_system.hpp"

namespace EncryptPad
{
#if defined(__MINGW__) || defined(__MINGW32__)
    const char *kRepositoryDirName = "_encryptpad";
#else
    const char *kRepositoryDirName = ".encryptpad";
#endif

    std::string repository_path;

    std::string GetRepositoryPath()
    {
        if(repository_path.empty())
        {

        }
    }
}

