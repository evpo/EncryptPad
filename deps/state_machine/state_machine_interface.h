//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <string>
#include "state_machine_utility.h"

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
    class StateMachineContext : public NonCopyable
    {
        protected:
            bool failed_;
        public:
            bool GetFailed() const;
            void SetFailed(bool value);
            StateMachineContext();
    };

    // Inherit this class to trace application specific state ids
    class StateIDToStringConverter: public NonCopyable
    {
        public:
            virtual std::string Convert(StateMachineStateID state_id) = 0;
            virtual std::string StateMachineName() = 0;
    };
}
