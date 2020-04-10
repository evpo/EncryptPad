//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "graph_builder.h"
#include <memory>
#include "state_handlers.h"
#include "state_id.h"

using namespace LightStateMachine;

namespace EncryptMsg
{
    LightStateMachine::StateGraph &BuildStateGraph()
    {
        static StateGraph sg;

        // Stub functions
        auto T = [](StateMachineContext &){ return true; };
        auto F = [](StateMachineContext &){ return false; };
        auto Stub = [](StateMachineContext &){};

        sg.Create(StateID::Start);
        sg.Create(StateID::Fail, Stub, Stub, T, F);

        sg.SetStartStateID(StateID::Start);
        sg.SetFailStateID(StateID::Fail);

        sg.Create(StateID::Init, InitOnEnter, Stub, InitCanEnter, T);
        sg.Create(StateID::End, Stub, Stub, EndCanEnter, F);
        sg.Create(StateID::Packet, PacketOnEnter, Stub, PacketCanEnter, PacketCanExit);
        sg.Create(StateID::FinishPacket, FinishOnEnter, Stub, FinishCanEnter, FinishCanExit);
        sg.Create(StateID::Header, HeaderOnEnter, Stub, HeaderCanEnter, T);
        sg.Create(StateID::Armor, ArmorOnEnter, Stub, ArmorCanEnter, T);

        sg.Link(StateID::Start, StateID::Init);
        sg.Link(StateID::Init, StateID::Armor);
        sg.Link(StateID::Armor, StateID::Packet);
        sg.Link(StateID::Armor, StateID::Header);
        sg.Link(StateID::Header, StateID::Packet);
        sg.Link(StateID::Packet, StateID::Header);
        sg.Link(StateID::Packet, StateID::FinishPacket);
        sg.Link(StateID::Packet, StateID::Packet);
        sg.Link(StateID::FinishPacket, StateID::Packet);
        sg.Link(StateID::FinishPacket, StateID::Header);
        sg.Link(StateID::FinishPacket, StateID::End);

        // BufferEmpty
        sg.Create(StateID::BufferEmpty, BufferEmptyOnEnter, Stub, BufferEmptyCanEnter, T);
        sg.Link(StateID::BufferEmpty, StateID::Header);
        sg.Link(StateID::BufferEmpty, StateID::End);
        sg.Link(StateID::Header, StateID::BufferEmpty);
        sg.Link(StateID::Packet, StateID::BufferEmpty);
        sg.Link(StateID::Armor, StateID::BufferEmpty);

        return sg;
    }
}

