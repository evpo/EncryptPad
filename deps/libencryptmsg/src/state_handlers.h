//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
//
// Handlers to various events occurring in the state machine
//
#pragma once
#include "context.h"

namespace EncryptMsg
{
    bool InitCanEnter(LightStateMachine::StateMachineContext &context);
    void InitOnEnter(LightStateMachine::StateMachineContext &context);

    bool EndCanEnter(LightStateMachine::StateMachineContext &context);

    void HeaderOnEnter(LightStateMachine::StateMachineContext &context);
    void PacketOnEnter(LightStateMachine::StateMachineContext &context);
    void BufferEmptyOnEnter(LightStateMachine::StateMachineContext &context);

    bool HeaderCanEnter(LightStateMachine::StateMachineContext &context);
    bool PacketCanEnter(LightStateMachine::StateMachineContext &context);
    bool PacketCanExit(LightStateMachine::StateMachineContext &context);

    bool FinishCanEnter(LightStateMachine::StateMachineContext &context);
    bool FinishCanExit(LightStateMachine::StateMachineContext &context);
    void FinishOnEnter(LightStateMachine::StateMachineContext &context);

    bool BufferEmptyCanEnter(LightStateMachine::StateMachineContext &context);
}
