//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "context.h"

namespace LibEncryptMsg
{
    Context::Context():
        session_state_(nullptr)
    {
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
