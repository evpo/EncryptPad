//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "state_handlers.h"
#include <algorithm>
#include "plog/Log.h"
#include "session_state.h"
#include "emsg_constants.h"

using namespace std;
using namespace EncryptMsg;
using namespace LightStateMachine;

namespace
{
    inline Context &ToContext(StateMachineContext &ctx)
    {
        StateMachineContext *p = &ctx;
        return *(static_cast<Context*>(p));
    }
}

namespace EncryptMsg
{
    bool InitCanEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        return !context.State().buffer_stack.empty() || context.State().finish_packets;
    }

    void InitOnEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        auto &state = context.State();
        state.packet_chain_it = state.packet_chain.begin();
    }

    bool HeaderCanEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        if(context.State().buffer_stack.empty() ||
                context.State().buffer_stack.top().empty())
            return false;

        if(context.State().packet_chain_it != context.State().packet_chain.end() &&
                *context.State().packet_chain_it != PacketType::Unknown)
            return false;

        return true;
    }

    bool HeaderCanExit(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        auto &state = context.State();
        switch(state.emsg_result)
        {
            case EmsgResult::Success:
            case EmsgResult::Pending:
                return true;
            default:
                return false;
        }
    }

    void HeaderOnEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        auto &state = context.State();
        auto &reader = state.packet_factory.GetHeaderReader();
        reader.GetInStream().Push(state.buffer_stack.top());
        state.buffer_stack.pop();
        state.emsg_result = reader.Read(state.finish_packets);
        switch(state.emsg_result)
        {
            case EmsgResult::Success:
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
            case EmsgResult::Pending:
                break;
            default:
                context.SetFailed(true);
                break;
        }
    }


    bool PacketCanExit(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        auto result = context.State().emsg_result;
        switch(result)
        {
            case EmsgResult::Success:
            case EmsgResult::Pending:
                return true;
            default:
                return false;
        }
    }

    bool PacketCanEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        if(context.State().buffer_stack.empty() && !context.State().finish_packets)
            return false;

        if(context.State().packet_chain_it == context.State().packet_chain.end() ||
                *context.State().packet_chain_it == PacketType::Unknown)
            return false;

        return true;
    }

    void PacketOnEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
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
        auto out_stm = EncryptMsg::MakeOutStream(output);

        // it can be empty when finishing
        if(!buffer_stack.empty())
        {
            packet.GetInStream().Push(buffer_stack.top());
            buffer_stack.pop();
        }

        state.emsg_result = packet.Read(*out_stm);
        switch(state.emsg_result)
        {
            case EmsgResult::Success:
                *state.packet_chain_it = PacketType::Unknown;
                if(packet.GetInStream().GetCount() > 0)
                {
                    buffer_stack.emplace();
                    AppendToBuffer(packet.GetInStream(), buffer_stack.top());
                }
                break;
            case EmsgResult::Pending:
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
        else if (state.emsg_result == EmsgResult::Success)
        {
            auto it = state.packet_chain.begin();
            for(;it != state.packet_chain.end() && *it != PacketType::Unknown; it++)
            {
            }
            state.packet_chain_it = it;
        }
    }

    bool FinishCanEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        if(*context.State().packet_chain_it == PacketType::Unknown)
            return false;

        return context.State().finish_packets;
    }

    bool FinishCanExit(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        return context.State().emsg_result == EmsgResult::Success;
    }

    void FinishOnEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        auto &state = context.State();
        auto packet_pair = state.packet_factory.GetOrCreatePacket(*state.packet_chain_it);
        assert(packet_pair.first);
        assert(!packet_pair.second);

        state.emsg_result = packet_pair.first->Finish();
        *state.packet_chain_it = PacketType::Unknown;
        state.packet_chain_it ++;
    }

    bool BufferEmptyCanEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        return context.State().buffer_stack.empty() || !context.State().finish_packets;
    }

    void BufferEmptyOnEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        auto &packet_chain = context.State().packet_chain;
        context.State().packet_chain_it = *packet_chain.begin() == PacketType::Unknown
            ? packet_chain.end() : packet_chain.begin();
        if(!context.State().buffer_stack.empty())
            context.State().buffer_stack.pop();
    }

    bool EndCanEnter(StateMachineContext &ctx)
    {
        Context &context = ToContext(ctx);
        auto &state = context.State();
        if(state.finish_packets && !std::all_of(state.packet_chain.begin(), state.packet_chain.end(),
                    [](PacketType packet_type)
                    {
                        return packet_type == PacketType::Unknown;
                    }))
        {
            return false;
        }

        return state.buffer_stack.empty();
    }
}
