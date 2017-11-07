#include "state_handlers.h"
#include <algorithm>
#include "plog/Log.h"
#include "session_state.h"
#include "emsg_constants.h"

using namespace std;
using namespace LightStateMachine;
using namespace LightStateMachine::Client;

namespace LibEncryptMsg
{
    bool InitCanEnter(Context &context)
    {
        return !context.State().buffer_stack.empty() || context.State().finish_packets;
    }

    void InitOnEnter(Context &context)
    {
        auto &state = context.State();
        state.packet_chain_it = state.packet_chain.begin();
    }

    bool HeaderCanEnter(Context &context)
    {
        if(context.State().buffer_stack.empty() ||
                context.State().buffer_stack.top().empty())
            return false;

        if(context.State().packet_chain_it != context.State().packet_chain.end() &&
                *context.State().packet_chain_it != PacketType::Unknown)
            return false;

        return true;
    }

    bool HeaderCanExit(Context &context)
    {
        auto &state = context.State();
        switch(state.packet_result)
        {
            case PacketResult::Success:
            case PacketResult::Pending:
                return true;
            default:
                return false;
        }
    }

    void HeaderOnEnter(Context &context)
    {
        auto &state = context.State();
        auto &reader = state.packet_factory.GetHeaderReader();
        reader.GetInStream().Push(state.buffer_stack.top());
        state.buffer_stack.pop();
        state.packet_result = reader.Read(state.finish_packets);
        switch(state.packet_result)
        {
            case PacketResult::Success:
                {
                    if(reader.GetInStream().GetCount() > 0)
                    {
                        state.buffer_stack.emplace();
                        AppendToBuffer(reader.GetInStream(), state.buffer_stack.top());
                    }
                    *state.packet_chain_it = reader.GetPacketHeader().packet_type;
                    LOG_DEBUG << "Header: " << GetPacketSpec(reader.GetPacketHeader().packet_type).packet_name;
                }
                break;
            case PacketResult::Pending:
                break;
            default:
                context.SetFailed(true);
                break;
        }
    }


    bool PacketCanExit(Context &context)
    {
        auto result = context.State().packet_result;
        switch(result)
        {
            case PacketResult::Success:
            case PacketResult::Pending:
                return true;
            default:
                return false;
        }
    }

    bool PacketCanEnter(Context &context)
    {
        if(context.State().buffer_stack.empty() && !context.State().finish_packets)
            return false;

        if(context.State().packet_chain_it == context.State().packet_chain.end() ||
                *context.State().packet_chain_it == PacketType::Unknown)
            return false;

        return true;
    }

    void PacketOnEnter(Context &context)
    {
        auto &state = context.State();
        auto packet_chain_it = state.packet_chain_it;
        LOG_DEBUG << "Packet: " << GetPacketSpec(*packet_chain_it).packet_name;
        auto packet_pair = state.packet_factory.GetOrCreatePacket(*packet_chain_it);
        assert(packet_pair.first);
        auto &packet = *packet_pair.first;

        if(packet_pair.second)
        {
            // new packet
            auto &packet_header = state.packet_factory.GetHeaderReader().GetPacketHeader();
            assert(packet_header.packet_type == *packet_chain_it);

            size_t body_length = packet_header.body_length;

            if(packet_header.is_partial_length && !packet_header.is_new_format)
            {
                body_length = kMaxStreamLength;
            }
            else if(packet_header.is_partial_length && packet_header.is_new_format)
            {
                packet.GetInStream().SetPartialLength(true, body_length);
            }
        }

        auto &buffer_stack = state.buffer_stack;
        Botan::secure_vector<uint8_t> output;
        auto out_stm = LibEncryptMsg::MakeOutStream(output);

        // it can be empty when finishing
        if(!buffer_stack.empty())
        {
            packet.GetInStream().Push(buffer_stack.top());
            buffer_stack.pop();
        }

        state.packet_result = packet.Read(*out_stm);
        switch(state.packet_result)
        {
            case PacketResult::Success:
                *state.packet_chain_it = PacketType::Unknown;
                if(packet.GetInStream().GetCount() > 0)
                {
                    buffer_stack.emplace();
                    AppendToBuffer(packet.GetInStream(), buffer_stack.top());
                }
                break;
            case PacketResult::Pending:
                break;
            default:
                context.SetFailed(true);
                return;
        }

        if(packet.IsFinalPacket() && !output.empty())
        {
            state.output.insert(state.output.end(), output.begin(), output.end());
        }
        else if(!output.empty())
        {
            buffer_stack.push(move(output));
            // if finishing, we need to enter finish packet state before moving to the next packet
            if(!state.finish_packets)
                state.packet_chain_it++;
            assert(state.packet_chain.end() != state.packet_chain_it);
        }
        else if (state.packet_result == PacketResult::Success)
        {
            auto it = state.packet_chain.begin();
            for(;it != state.packet_chain.end() && *it != PacketType::Unknown; it++)
            {
            }
            state.packet_chain_it = it;
        }
    }

    bool FinishCanEnter(Context &context)
    {
        if(*context.State().packet_chain_it == PacketType::Unknown)
            return false;

        return context.State().finish_packets;
    }

    bool FinishCanExit(Context &context)
    {
        return context.State().packet_result == PacketResult::Success;
    }

    void FinishOnEnter(Context &context)
    {
        auto &state = context.State();
        auto packet_pair = state.packet_factory.GetOrCreatePacket(*state.packet_chain_it);
        assert(packet_pair.first);
        assert(!packet_pair.second);

        state.packet_result = packet_pair.first->Finish();
        *state.packet_chain_it = PacketType::Unknown;
        state.packet_chain_it ++;
    }

    bool BufferEmptyCanEnter(Context &context)
    {
        return context.State().buffer_stack.empty() || !context.State().finish_packets;
    }

    void BufferEmptyOnEnter(Context &context)
    {
        auto &packet_chain = context.State().packet_chain;
        context.State().packet_chain_it = *packet_chain.begin() == PacketType::Unknown
            ? packet_chain.end() : packet_chain.begin();
        if(!context.State().buffer_stack.empty())
            context.State().buffer_stack.pop();
    }

    bool EndCanEnter(Context &context)
    {
        auto &state = context.State();
        if(state.finish_packets && *state.packet_chain_it != PacketType::Unknown)
            return false;

        return state.buffer_stack.empty();
    }
}
