#pragma once
#include <string>

namespace LightStateMachine
{
    namespace Client
    {
        enum class StateID
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
}
