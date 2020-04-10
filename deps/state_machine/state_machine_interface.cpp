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

    bool StateMachineContext::IsReentry() const
    {
        return is_reentry_;
    }

    // It's for internal use from the state machine
    void StateMachineContext::SetIsReentry(bool is_reentry)
    {
        is_reentry_ = is_reentry;
    }

    StateMachineContext::StateMachineContext():
        failed_(false),
        is_reentry_(false)
    {
    }
}
