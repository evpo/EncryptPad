#pragma once
#include <type_traits>
#include <utility>

namespace LibEncryptMsg
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
