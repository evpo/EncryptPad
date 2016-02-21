//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//This file is part of EncryptPad
//
//EncryptPad is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 2 of the License, or
//(at your option) any later version.
//
//EncryptPad is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EncryptPad.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************
#pragma once
#include <string>
#include <cstdio>

namespace EncryptPad
{
    class NonCopyableNonMovable
    {
        NonCopyableNonMovable(const NonCopyableNonMovable&) = delete;
        const NonCopyableNonMovable &operator=(const NonCopyableNonMovable&) = delete;
        NonCopyableNonMovable(const NonCopyableNonMovable&&) = delete;
        const NonCopyableNonMovable &operator=(const NonCopyableNonMovable&&) = delete;

    public:
        NonCopyableNonMovable() = default;
    };

    template<typename F, typename DecayF=typename std::decay<F>::type>
    class CleanUp : private NonCopyableNonMovable
    {
        DecayF f_;
    public:
        template<typename T>
        CleanUp(T &&f)
            :f_(std::forward<T>(f))
        {}

        ~CleanUp()
        {
            f_();
        }
    };

    bool LoadStringFromFile(const std::string &file_name, std::string &content);

    template<typename Handle, typename Traits>
    class UniqueHandler
    {
    private:
        Handle handle_;
    public:
        UniqueHandler(Handle handle = Traits::InvalidHandle()):handle_(handle)
        {
        }

        UniqueHandler(UniqueHandler &&uh)
        {
            handle_ = uh.handle_;
            uh.handle_ = Traits::InvalidHandle();
        }

        ~UniqueHandler()
        {
            if(handle_ == Traits::InvalidHandle())
                return;
            Traits::Close(handle_);
        }

        UniqueHandler(UniqueHandler &uh)
        {
            handle_ = uh.handle_;
            uh.handle_ = Traits::InvalidHandle();
        }

        const UniqueHandler &operator=(UniqueHandler &uh)
        {
            handle_ = uh.handle_;
            uh.handle_ = Traits::InvalidHandle();
            return *this;
        }

        Handle get() const
        {
            return handle_;
        }

        explicit operator bool() const
        {
            return handle_ != Traits::InvalidHandle();
        }

        bool Valid() const
        {
            return operator bool();
        }
    };

    class FileTraits
    {
    public:
        static FILE *InvalidHandle()
        {
            return nullptr;
        }

        static void Close(FILE *f)
        {
            fclose(f);
        }
    };

    typedef UniqueHandler<FILE *, FileTraits> FileHndl; 
}
