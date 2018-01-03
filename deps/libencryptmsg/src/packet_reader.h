#pragma once
#include <memory>
#include <map>
#include <utility>
#include "memory_stream.h"
#include "packet_parsers.h"
#include "emsg_types.h"

namespace LibEncryptMsg
{
    struct SessionState;

    class PacketRWBase
    {
    protected:
        InBufferStream in_;
        SessionState &state_;
        bool is_final_packet_;

        virtual EmsgResult DoRead(OutStream &out) = 0;
        virtual EmsgResult DoFinish() = 0;
    public:
        PacketRWBase(SessionState &state, bool is_final_packet)
            :state_(state), is_final_packet_(is_final_packet){}
        // Reads packet body and writes its payload while setting metadata_out fields
        // Payload is the contained packets or empty if there are no packets inside
        EmsgResult Read(OutStream &out)
        {
            return DoRead(out);
        }

        EmsgResult Finish()
        {
            return DoFinish();
        }

        InBufferStream &GetInStream()
        {
            return in_;
        }

        bool IsFinalPacket()
        {
            return is_final_packet_;
        }
    };

    class HeaderReader
    {
        private:
            PacketHeader packet_header_;
            InBufferStream in_stm_;
        public:
            InBufferStream &GetInStream()
            {
                return in_stm_;
            }

            EmsgResult Read(bool finish_packets);

            PacketHeader &GetPacketHeader()
            {
                return packet_header_;
            }
    };

    class PacketFactory
    {
        private:
            HeaderReader header_reader_;
            std::map<PacketType, std::unique_ptr<PacketRWBase> > packet_map_;
            SessionState &session_state_;
        public:
            PacketFactory(SessionState &session_state);

            // Get or create a packet reader. Returns a pointer to the reader and true if the packet was created
            std::pair<PacketRWBase*, bool> GetOrCreatePacket(PacketType packet_type);
            HeaderReader &GetHeaderReader()
            {
                return header_reader_;
            }
    };
}
