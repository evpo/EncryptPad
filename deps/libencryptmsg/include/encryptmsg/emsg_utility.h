//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <type_traits>
#include <utility>

namespace EncryptMsg
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
}
