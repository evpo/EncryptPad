#pragma once
#include <functional>
#include "state_machine_interface.h"

namespace LightStateMachine
{
    void StubVoidFunction(StateMachineContext&);
    bool StubBoolFunction(StateMachineContext&);

    using VoidFunction = std::function<void(StateMachineContext&)>;
    using BoolFunction = std::function<bool(StateMachineContext&)>;

    // State machine state
    // Copy semantic
    class State
    {
        public:
            State(StateMachineStateID id,
                    VoidFunction on_enter = StubVoidFunction,
                    VoidFunction on_exit = StubVoidFunction,
                    BoolFunction can_enter = StubBoolFunction,
                    BoolFunction can_exit = StubBoolFunction);
            StateMachineStateID GetID() const;
            bool CanEnter(StateMachineContext &context);
            bool CanExit(StateMachineContext &context);
            void OnEnter(StateMachineContext &context);
            void OnExit(StateMachineContext &context);

        private:
            StateMachineStateID state_id_;
            VoidFunction on_enter_;
            VoidFunction on_exit_;
            BoolFunction can_enter_;
            BoolFunction can_exit_;
    };
}
