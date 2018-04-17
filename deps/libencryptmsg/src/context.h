//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include "state_machine_utility.h"
#include "state_machine_interface.h"

namespace EncryptMsg
{
    struct SessionState;

    // Context class provides state handlers with access to the input and output
    class Context : public LightStateMachine::StateMachineContext
    {
        private:
            SessionState *session_state_;
        public:
            Context();
            EncryptMsg::SessionState &State();
            void SetState(EncryptMsg::SessionState &session_state);
    };
}
