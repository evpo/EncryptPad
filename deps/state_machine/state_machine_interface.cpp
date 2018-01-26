//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "state_machine_interface.h"

namespace LightStateMachine
{
    bool StateMachineContext::GetFailed() const
    {
        return failed_;
    }
    void StateMachineContext::SetFailed(bool value)
    {
        failed_ = value;
    }
    StateMachineContext::StateMachineContext():
        failed_(false)
    {
    }
}
