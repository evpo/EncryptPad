//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "state_graph.h"

namespace LightStateMachine
{
    State &StateGraph::DoCreate(StateMachineStateID state_id,
            VoidFunction on_enter,
            VoidFunction on_exit,
            BoolFunction can_enter,
            BoolFunction can_exit)
    {
        auto it = graph_.insert(State(state_id, on_enter, on_exit, can_enter, can_exit));
        states_[state_id] = it;
        return *it;
    }

    State &StateGraph::DoGet(StateMachineStateID state_id)
    {
        return *states_[state_id];
    }

    void StateGraph::DoLink(StateMachineStateID left, StateMachineStateID right)
    {
        graph_.arc_insert(states_[left], states_[right]);
    }

    StateGraph::StateGraph()
    {
    }
    StateGraph::~StateGraph()
    {
    }
}
