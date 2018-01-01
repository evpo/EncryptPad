#pragma once
#include "state_machine_utility.h"
#include "state_machine_interface.h"

namespace LibEncryptMsg
{
    class SessionState;

    // Context class provides state handlers with access to the input and output
    class Context : public LightStateMachine::StateMachineContext
    {
        private:
            SessionState *session_state_;
        public:
            Context();
            LibEncryptMsg::SessionState &State();
            void SetState(LibEncryptMsg::SessionState &session_state);
    };
}
