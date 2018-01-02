#include "decryption_state_debug.h"
#include "decryption_state_machine.h"

using namespace EncryptPad;
std::string PrintDecryptionStateMachineStateID(LightStateMachine::StateMachineStateID state_id)
{
    switch(static_cast<StateID>(state_id))
    {
        case StateID::Start:
            return "Start";
        case StateID::End:
            return "End";
        case StateID::ReadIn:
            return "ReadIn";
        case StateID::ParseFormat:
            return "ParseFormat";
        case StateID::SetPwdKey:
            return "SetPwdKey";
        case StateID::ReadKeyFile:
            return "ReadKeyFile";
        case StateID::GPG:
            return "GPG";
        case StateID::WADHead:
            return "WADHead";
        case StateID::WriteOut:
            return "WriteOut";
        case StateID::Fail:
            return "Fail";
        default:
            return "Unknown state";
    }
}

