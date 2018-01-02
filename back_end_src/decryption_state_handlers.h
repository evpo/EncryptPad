#pragma once
#include "decryption_state_machine.h"
#include "state_machine_interface.h"

namespace EncryptPad
{
    bool ReadIn_CanEnter(LightStateMachine::StateMachineContext &ctx);
    bool ParseFormat_CanEnter(LightStateMachine::StateMachineContext &ctx);
    bool SetPwdKey_CanEnter(LightStateMachine::StateMachineContext &ctx);
    bool ReadKeyFile_CanEnter(LightStateMachine::StateMachineContext &ctx);
    bool GPG_CanEnter(LightStateMachine::StateMachineContext &ctx);
    bool WADHead_CanEnter(LightStateMachine::StateMachineContext &ctx);
    bool WriteOut_CanEnter(LightStateMachine::StateMachineContext &ctx);
    bool End_CanEnter(LightStateMachine::StateMachineContext &ctx);

    void ReadIn_OnEnter(LightStateMachine::StateMachineContext &ctx);
    void ParseFormat_OnEnter(LightStateMachine::StateMachineContext &ctx);
    void SetPwdKey_OnEnter(LightStateMachine::StateMachineContext &ctx);
    void ReadKeyFile_OnEnter(LightStateMachine::StateMachineContext &ctx);
    void GPG_OnEnter(LightStateMachine::StateMachineContext &ctx);
    void WADHead_OnEnter(LightStateMachine::StateMachineContext &ctx);
    void WriteOut_OnEnter(LightStateMachine::StateMachineContext &ctx);
    void Fail_OnEnter(LightStateMachine::StateMachineContext &ctx);
    void End_OnEnter(LightStateMachine::StateMachineContext &ctx);
}

