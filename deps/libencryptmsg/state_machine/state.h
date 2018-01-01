#pragma once
#include "context.h"
#include "state_id.h"
#include <functional>

namespace LightStateMachine
{
    void StubVoidFunction(Client::Context&);
    bool StubBoolFunction(Client::Context&);

    using VoidFunction = std::function<void(Client::Context&)>;
    using BoolFunction = std::function<bool(Client::Context&)>;

    // State machine state
    // Copy semantic
    class State
    {
        public:
            State(Client::StateID id,
                    VoidFunction on_enter = StubVoidFunction,
                    VoidFunction on_exit = StubVoidFunction,
                    BoolFunction can_enter = StubBoolFunction,
                    BoolFunction can_exit = StubBoolFunction);
            Client::StateID GetID() const;
            bool CanEnter(Client::Context &context);
            bool CanExit(Client::Context &context);
            void OnEnter(Client::Context &context);
            void OnExit(Client::Context &context);

        private:
            Client::StateID state_id_;
            VoidFunction on_enter_;
            VoidFunction on_exit_;
            BoolFunction can_enter_;
            BoolFunction can_exit_;
    };
}
