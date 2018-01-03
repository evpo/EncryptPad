#include "session_state.h"
#include "emsg_constants.h"

namespace LibEncryptMsg
{
    SessionState::SessionState()
        :packet_chain(kMaxPacketChainLength, PacketType::Unknown),
        packet_chain_it(packet_chain.end()), key_provider(nullptr),
        packet_factory(*this), emsg_result(EmsgResult::None), is_message_analyzed(false),
        finish_packets(false)
    {
    }
}
