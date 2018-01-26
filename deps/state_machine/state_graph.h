//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include "digraph.hpp"
#include "state.h"

namespace LightStateMachine
{
    typedef stlplus::digraph<State,bool> StateGraph;
}
