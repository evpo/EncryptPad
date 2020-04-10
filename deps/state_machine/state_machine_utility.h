//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <stdexcept>

namespace LightStateMachine
{
    class NonCopyable
    {
        public:
            NonCopyable(){}
            NonCopyable(const NonCopyable &) = delete;
            const NonCopyable &operator=(const NonCopyable&) = delete;
    };

    template<class T>
    class OptionalValue
    {
        private:
            bool set_ = false;
            T value_ = -1;
        public:
            void Set(T value)
            {
                value_ = value;
                set_ = true;
            }

            bool IsSet() const
            {
                return set_;
            }

            T Get() const
            {
                if(!set_)
                {
                    throw std::runtime_error("The value is not set");
                }
                return value_;
            }
    };

}
