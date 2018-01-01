#pragma once
#include "state_machine_utility.h"

namespace LibEncryptMsg
{
    class SessionState;
}

namespace LightStateMachine
{
    namespace Client
    {
        // Context class provides state handlers with access to the input and output
        class Context : public NonCopyable
        {
            private:
                bool failed_;
                LibEncryptMsg::SessionState *session_state_;
            public:
                Context();
                bool GetFailed() const;
                void SetFailed(bool value);
                LibEncryptMsg::SessionState &State();
                void SetState(LibEncryptMsg::SessionState &session_state);
        };
    }
}
