#include "state.h"

using namespace LightStateMachine::Client;

namespace LightStateMachine
{

    State::State(StateID id, VoidFunction on_enter, VoidFunction on_exit,
            BoolFunction can_enter, BoolFunction can_exit)
        :
        state_id_(id),
        on_enter_(on_enter),
        on_exit_(on_exit),
        can_enter_(can_enter),
        can_exit_(can_exit)
    {
    }

    StateID State::GetID() const
    {
        return state_id_;
    }

    bool State::CanEnter(Context &context)
    {
        return can_enter_(context);
    }

    bool State::CanExit(Context &context)
    {
        return can_exit_(context);
    }

    void State::OnEnter(Context &context)
    {
        on_enter_(context);
    }

    void State::OnExit(Context &context)
    {
        on_exit_(context);
    }

    void StubVoidFunction(Client::Context&)
    {
    }

    bool StubBoolFunction(Client::Context&)
    {
        return true;
    }
}
