//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include "state_graph.h"

namespace EncryptMsg
{
    // The graph of states is built here.
    LightStateMachine::StateGraph &BuildStateGraph();
}

