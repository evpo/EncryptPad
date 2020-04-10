//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <string>
#include "state_machine_interface.h"

namespace EncryptMsg
{
    enum class StateID : int
    {
        Start,
        Init,
        Armor,
        Header,
        Packet,
        FinishPacket,
        BufferEmpty,
        End,
        Fail
    };
    const std::string &PrintStateID(StateID state_id);
    std::string EmsgStateIDToStringConverter(LightStateMachine::StateMachineStateID state_id);
}
