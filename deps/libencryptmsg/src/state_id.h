//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <string>
#include "state_machine_interface.h"

namespace LibEncryptMsg
{
    enum class StateID : int
    {
        Start,
        Init,
        Header,
        Packet,
        FinishPacket,
        BufferEmpty,
        End,
        Fail
    };
    const std::string &PrintStateID(StateID state_id);

    class EmsgStateIDToStringConverter : public LightStateMachine::StateIDToStringConverter
    {
        public:
            std::string Convert(LightStateMachine::StateMachineStateID state_id)
            {
                return PrintStateID(static_cast<StateID>(state_id));
            }

            std::string StateMachineName()
            {
                return "emsg";
            }
    };
}
