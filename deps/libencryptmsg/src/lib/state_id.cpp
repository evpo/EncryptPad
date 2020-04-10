//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "state_id.h"
#include <map>

using namespace std;
namespace EncryptMsg
{
    const std::string &PrintStateID(StateID state_id)
    {
        static map<StateID, string> state_id2string =
        {
            {StateID::Start, "Start"},
            {StateID::Init, "Init"},
            {StateID::Armor, "Armor"},
            {StateID::Header,"Header"},
            {StateID::Packet,"Packet"},
            {StateID::BufferEmpty, "BufferEmpty"},
            {StateID::FinishPacket, "FinishPacket"},
            {StateID::End, "End"},
            {StateID::Fail, "Fail"}
        };
        return state_id2string[state_id];
    }

    std::string EmsgStateIDToStringConverter(LightStateMachine::StateMachineStateID state_id)
    {
        return string("emsg:") + PrintStateID(static_cast<StateID>(state_id));
    }
}
