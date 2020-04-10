//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "state.h"

namespace LightStateMachine
{

    StateMachineStateID State::GetID() const
    {
        return state_id_;
    }

    bool State::CanEnter(StateMachineContext &context)
    {
        return can_enter_(context);
    }

    bool State::CanExit(StateMachineContext &context)
    {
        return can_exit_(context);
    }

    void State::OnEnter(StateMachineContext &context)
    {
        on_enter_(context);
    }

    void State::OnExit(StateMachineContext &context)
    {
        on_exit_(context);
    }

    void State::SetCanEnter(BoolFunction can_enter)
    {
        can_enter_ = can_enter;
    }

    void State::SetCanExit(BoolFunction can_exit)
    {
        can_exit_ = can_exit;
    }

    void State::SetOnEnter(VoidFunction on_enter)
    {
        on_enter_ = on_enter;
    }

    void State::SetOnExit(VoidFunction on_exit)
    {
        on_exit_ = on_exit;
    }

    void StubVoidFunction(StateMachineContext&)
    {
    }

    bool StubBoolFunction(StateMachineContext&)
    {
        return true;
    }

    bool AlwaysFalseBoolFunction(StateMachineContext&)
    {
        return false;
    }
}
