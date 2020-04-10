//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <map>
#include "digraph.hpp"
#include "state.h"
#include "utility.h"
#include "state_machine_utility.h"
#include "state_machine.h"

namespace LightStateMachine
{
    class StateGraph final : public NonCopyable
    {
        private:
            stlplus::digraph<State,bool> graph_;
            std::map<StateMachineStateID, stlplus::digraph<State,bool>::iterator> states_;
            OptionalValue<StateMachineStateID> start_state_id_;
            OptionalValue<StateMachineStateID> fail_state_id_;

            stlplus::digraph<State,bool>::iterator fail_;

            State &DoCreate(StateMachineStateID state_id,
                    VoidFunction on_enter = StubVoidFunction,
                    VoidFunction on_exit = StubVoidFunction,
                    BoolFunction can_enter = StubBoolFunction,
                    BoolFunction can_exit = StubBoolFunction);
            State &DoGet(StateMachineStateID state_id);
            void DoLink(StateMachineStateID left, StateMachineStateID right);
        public:
            StateGraph();
            ~StateGraph();

            template<class T>
            State &Get(T id)
            {
                return DoGet(static_cast<StateMachineStateID>(id));
            }

            template<class T>
            State &Create(T id,
                    VoidFunction on_enter = StubVoidFunction,
                    VoidFunction on_exit = StubVoidFunction,
                    BoolFunction can_enter = StubBoolFunction,
                    BoolFunction can_exit = StubBoolFunction)
            {
                return DoCreate(static_cast<StateMachineStateID>(id),
                        on_enter, on_exit, can_enter, can_exit);
            }

            template<class T>
            void Link(T left, T right)
            {
                DoLink(static_cast<StateMachineStateID>(left),
                        static_cast<StateMachineStateID>(right));
            }

            template<class T>
            void SetStartStateID(T state_id)
            {
                start_state_id_.Set(static_cast<StateMachineStateID>(state_id));
            }

            template<class T>
            void SetFailStateID(T state_id)
            {
                fail_state_id_.Set(static_cast<StateMachineStateID>(state_id));
            }

            // for internal use
            stlplus::digraph<State,bool> &GetGraph()
            {
                return graph_;
            }

            // for internal use
            std::map<StateMachineStateID, stlplus::digraph<State,bool>::iterator> &StatesMap()
            {
                return states_;
            }

            StateMachineStateID GetStartStateID() const
            {
                return start_state_id_.Get();
            }

            StateMachineStateID GetFailStateID() const
            {
                return fail_state_id_.Get();
            }
    };
}
