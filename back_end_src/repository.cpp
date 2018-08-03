#include "repository.h"
#include "file_system.hpp"

namespace
{
#if defined(__MINGW__) || defined(__MINGW32__)
    const std::string repository_dir_name = "_encryptpad";
#else
    const std::string repository_dir_name = ".encryptpad";
#endif
    const std::string portable_repository_dir_name = "encryptpad_repository";

    std::string repository_path;
}

namespace EncryptPad
{
    void InitializeRepositoryPath(const std::string &arg0)
    {
        // check if the install directory contains the repository
        std::string portable_repository = stlplus::create_filespec(
                stlplus::install_path(arg0),
                portable_repository_dir_name);

        if(stlplus::folder_exists(portable_repository))
        {
            repository_path = portable_repository;
        }
        else
        {
            repository_path = stlplus::create_filespec(
                    stlplus::folder_user_profile(),
                    repository_dir_name);

            if(!stlplus::folder_exists(repository_path) && !stlplus::folder_create(repository_path))
            {
                repository_path.clear();
            }

        }
    }

    const std::string &GetRepositoryPath()
    {
        return repository_path;
    }
}

