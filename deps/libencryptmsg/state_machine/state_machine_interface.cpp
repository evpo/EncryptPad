#include "state_machine_interface.h"

std::string PrintStateMachineStateID(LightStateMachine::StateMachineStateID state_id)
{
    return std::string("State " + std::to_string(state_id));
}

namespace LightStateMachine
{
    bool StateMachineContext::GetFailed() const
    {
        return failed_;
    }
    void StateMachineContext::SetFailed(bool value)
    {
        failed_ = value;
    }
    StateMachineContext::StateMachineContext():
        failed_(false)
    {
    }
}
