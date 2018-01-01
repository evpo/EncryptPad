#pragma once
#include <string>
#include "state_machine_utility.h"

namespace LightStateMachine
{
    //The caller is recommended to use typed "enum class" and cast to int
    //before interacting with LightStateMachine as below:
    //enum class StateID : int
    //{
    //    Init,
    //    Stop,
    //};
    using StateMachineStateID = int;


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
}

//Define your own function to log your application specific state id
#ifndef PRINT_STATE_MACHINE_STATE_ID
    std::string PrintStateMachineStateID(LightStateMachine::StateMachineStateID state_id);
#define PRINT_STATE_MACHINE_STATE_ID PrintStateMachineStateID
#endif

