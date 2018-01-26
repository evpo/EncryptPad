//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once

#include <queue>
#include <memory>
#include "state_machine_interface.h"
#include "state_graph.h"
#include "state_machine_utility.h"

namespace LightStateMachine
{
    class StateMachine : private NonCopyable
    {
        public:
            StateMachine(StateGraph &state_graph, StateGraph::iterator start_state, StateGraph::iterator fail_state, StateMachineContext &context);
            bool NextState();
            StateMachineStateID CurrentState() const;
            StateMachineStateID PreviousState() const;
            void Reset();
            void SetStateIDToStringConverter(std::unique_ptr<StateIDToStringConverter> converter);
        private:
            void SetCurrentState(StateGraph::iterator new_state);
            StateGraph *state_graph_;
            StateGraph::iterator start_state_;
            StateGraph::iterator current_state_;
            StateGraph::iterator fail_state_;
            StateMachineContext *context_;
            std::queue<StateMachineStateID> state_queue_;
            std::unique_ptr<StateIDToStringConverter> state_id_to_string_converter_;
    };
}
