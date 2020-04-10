//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <string>
#include <functional>
#include "state_machine_utility.h"
#include "state_machine_type_erasure.h"

namespace LightStateMachine
{
    using StateMachineStateID = int;

    //StateID
    //The caller is recommended to use typed "enum class" and cast to int
    //before interacting with LightStateMachine as below:
    //enum class StateID : StateMachineStateID
    //{
    //    Init,
    //    Stop,
    //};

    //Inherit this class to add the application specific context
    //or use SetExtra and GetExtra<T> to attach it
    class StateMachineContext : public NonCopyable
    {
        private:
            bool failed_;
            bool is_reentry_;
            TypeErasure extra_context_;
        public:
            bool GetFailed() const;
            void SetFailed(bool value);

            // The flag is tested in CanEnter and OnEnter
            // to check if we reenter the current state
            bool IsReentry() const;

            // It's for internal use from the state machine
            void SetIsReentry(bool is_reentry);

            StateMachineContext();

            template<class T>
            StateMachineContext(T *extra_context):
                failed_(false),
                extra_context_(extra_context)
            {
            }

            template<class T>
            void SetExtra(T *extra_context)
            {
                extra_context_ = extra_context;
            }

            template<class T>
            T &Extra()
            {
                return extra_context_.Get<T>();
            }

            template<class T>
            T &Extra() const
            {
                return extra_context_.Get<T>();
            }
    };

    // Define this function type to trace application specific state ids
    using StateIDToStringConverter = std::function<std::string(StateMachineStateID)>;
}
