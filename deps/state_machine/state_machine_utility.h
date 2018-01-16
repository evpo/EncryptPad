#pragma once
namespace LightStateMachine
{
    class NonCopyable
    {
        public:
            NonCopyable(){}
            NonCopyable(const NonCopyable &) = delete;
            const NonCopyable &operator=(const NonCopyable&) = delete;
    };
}
