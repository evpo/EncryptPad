//
// Handlers to various events occurring in the state machine
//
#pragma once
#include "context.h"

#define Context LightStateMachine::Client::Context
namespace LibEncryptMsg
{
    bool InitCanEnter(Context &context);
    void InitOnEnter(Context &context);

    bool EndCanEnter(Context &context);

    void HeaderOnEnter(Context &context);
    void PacketOnEnter(Context &context);
    void BufferEmptyOnEnter(Context &context);

    bool HeaderCanEnter(Context &context);
    bool PacketCanEnter(Context &context);
    bool PacketCanExit(Context &context);

    bool FinishCanEnter(Context &context);
    bool FinishCanExit(Context &context);
    void FinishOnEnter(Context &context);

    bool BufferEmptyCanEnter(Context &context);
}
#undef Context
