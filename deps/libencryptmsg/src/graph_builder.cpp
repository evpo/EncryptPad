#include "graph_builder.h"
#include <memory>
#include "state_handlers.h"
#include "state_id.h"

using namespace LightStateMachine;

namespace LibEncryptMsg
{
    int ToInt(StateID state_id)
    {
        return static_cast<int>(state_id);
    }

    StateGraphInfo BuildStateGraph()
    {
        static StateGraph state_graph;
        static StateGraph::iterator start_node;
        static StateGraph::iterator fail_node;
        if(!state_graph.empty())
        {
            return {
                &state_graph,
                start_node,
                fail_node
            };
        }

        // Insert new state
        auto I = [&](State s)->StateGraph::iterator
        {
            return state_graph.insert(s);
        };

        // Link states
        auto L = [&](StateGraph::iterator l, StateGraph::iterator r)
        {
            state_graph.arc_insert(l, r);
        };

        // Stub functions
        auto T = [](StateMachineContext &){ return true; };
        auto F = [](StateMachineContext &){ return false; };
        auto Stub = [](StateMachineContext &){};

        // Start state
        start_node = I(State(ToInt(StateID::Start)));

        // Fail state
        fail_node = I(State(ToInt(StateID::Fail), Stub, Stub, T, F));

        // Init state
        auto init_node = I(State(ToInt(StateID::Init), InitOnEnter, Stub, InitCanEnter, T));

        // End state
        auto end_node = I(State(ToInt(StateID::End), Stub, Stub, EndCanEnter, F));

        // Packet
        auto packet_node = I(State(ToInt(StateID::Packet), PacketOnEnter, Stub, PacketCanEnter, PacketCanExit));

        // FinishPacket
        auto finish_node = I(State(ToInt(StateID::FinishPacket), FinishOnEnter, Stub, FinishCanEnter, FinishCanExit));

        // Header
        auto header_node = I(State(ToInt(StateID::Header), HeaderOnEnter, Stub, HeaderCanEnter, T));

        L(start_node, init_node);
        L(init_node, packet_node);
        L(init_node, header_node);
        L(header_node, packet_node);
        L(packet_node, header_node);
        L(packet_node, finish_node);
        L(packet_node, packet_node);
        L(finish_node, packet_node);
        L(finish_node, end_node);

        // BufferEmpty
        auto buffer_empty_node = I(State(ToInt(StateID::BufferEmpty), BufferEmptyOnEnter, Stub, BufferEmptyCanEnter, T));
        L(buffer_empty_node, header_node);
        L(buffer_empty_node, end_node);
        L(header_node, buffer_empty_node);
        L(packet_node, buffer_empty_node);

        return {
            &state_graph,
            start_node,
            fail_node
        };
    }
}

