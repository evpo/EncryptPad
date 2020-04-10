//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once

#include <queue>
#include <memory>
#include "digraph.hpp"
#include "state_machine_interface.h"
#include "state_graph.h"
#include "state_machine_utility.h"

namespace LightStateMachine
{
    using DigraphIt = stlplus::digraph<State,bool>::iterator;
    class StateGraph;

    class StateMachine : private NonCopyable
    {
        public:
            StateMachine(StateGraph &state_graph, StateMachineContext &context);
            bool NextState();
            StateMachineStateID CurrentState() const;
            StateMachineStateID PreviousState() const;
            void Reset();
            void SetStateIDToStringConverter(StateIDToStringConverter converter);
        private:
            StateGraph *state_graph_;
            DigraphIt start_state_;
            DigraphIt current_state_;
            DigraphIt fail_state_;
            bool is_first_entry_;
            StateMachineContext *context_;
            std::queue<StateMachineStateID> state_queue_;
            StateIDToStringConverter state_id_to_string_converter_;

            void SetCurrentState(DigraphIt new_state);
    };
}
