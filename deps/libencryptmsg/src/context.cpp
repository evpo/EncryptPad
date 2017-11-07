#include "context.h"

using namespace LibEncryptMsg;

namespace LightStateMachine
{
    namespace Client
    {
        Context::Context():
            failed_(false), session_state_(nullptr)
        {
        }

        bool Context::GetFailed() const
        {
            return failed_;
        }

        void Context::SetFailed(bool value)
        {
            failed_ = value;
        }

        LibEncryptMsg::SessionState &Context::State()
        {
            return *session_state_;
        }
        void Context::SetState(LibEncryptMsg::SessionState &session_state)
        {
            session_state_ = &session_state;
        }
    }
}
