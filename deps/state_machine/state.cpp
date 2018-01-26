//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "state.h"

namespace LightStateMachine
{

    State::State(StateMachineStateID id, VoidFunction on_enter, VoidFunction on_exit,
            BoolFunction can_enter, BoolFunction can_exit)
        :
        state_id_(id),
        on_enter_(on_enter),
        on_exit_(on_exit),
        can_enter_(can_enter),
        can_exit_(can_exit)
    {
    }

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

    void StubVoidFunction(StateMachineContext&)
    {
    }

    bool StubBoolFunction(StateMachineContext&)
    {
        return true;
    }
}
