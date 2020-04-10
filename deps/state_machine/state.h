//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <functional>
#include "state_machine_interface.h"

namespace LightStateMachine
{
    void StubVoidFunction(StateMachineContext&);
    bool StubBoolFunction(StateMachineContext&);
    bool AlwaysFalseBoolFunction(StateMachineContext&);

    using VoidFunction = std::function<void(StateMachineContext&)>;
    using BoolFunction = std::function<bool(StateMachineContext&)>;

    // State machine state
    // Copy semantic
    class State
    {
        public:
            template<class T>
            State(T id,
                  VoidFunction on_enter = StubVoidFunction,
                  VoidFunction on_exit = StubVoidFunction,
                  BoolFunction can_enter = StubBoolFunction,
                  BoolFunction can_exit = StubBoolFunction):
                    state_id_(static_cast<StateMachineStateID>(id)),
                    on_enter_(on_enter),
                    on_exit_(on_exit),
                    can_enter_(can_enter),
                    can_exit_(can_exit)
            {
            }

            StateMachineStateID GetID() const;
            bool CanEnter(StateMachineContext &context);
            bool CanExit(StateMachineContext &context);
            void OnEnter(StateMachineContext &context);
            void OnExit(StateMachineContext &context);

            void SetCanEnter(BoolFunction can_enter);
            void SetCanExit(BoolFunction can_exit);
            void SetOnEnter(VoidFunction on_enter);
            void SetOnExit(VoidFunction on_exit);

        private:
            StateMachineStateID state_id_;
            VoidFunction on_enter_;
            VoidFunction on_exit_;
            BoolFunction can_enter_;
            BoolFunction can_exit_;
    };

    template<class T>
    struct VoidMemberFunction
    {
        using TMemberFunc = void (T::*)(StateMachineContext&);
        private:
            T *object_;
            TMemberFunc member_func_;
        public:
            VoidMemberFunction(T *object, TMemberFunc member_func):
                object_(object),
                member_func_(member_func)
            {
            }

            void operator()(StateMachineContext& context)
            {
                (object_->*member_func_)(context);
            }

    };

    template<class T>
    struct BoolMemberFunction
    {
        using TMemberFunc = bool (T::*)(StateMachineContext&);
        private:
            T *object_;
            TMemberFunc member_func_;
        public:
            BoolMemberFunction(T *object, TMemberFunc member_func):
                object_(object),
                member_func_(member_func)
            {
            }

            bool operator()(StateMachineContext& context)
            {
                return (object_->*member_func_)(context);
            }
    };

}
