//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "context.h"

namespace EncryptMsg
{
    Context::Context():
        session_state_(nullptr)
    {
    }

    EncryptMsg::SessionState &Context::State()
    {
        return *session_state_;
    }
    void Context::SetState(EncryptMsg::SessionState &session_state)
    {
        session_state_ = &session_state;
    }
}
