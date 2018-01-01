#pragma once
#include <string>

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
}
