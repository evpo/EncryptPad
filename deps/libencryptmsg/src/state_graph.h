#pragma once
#include "digraph.hpp"
#include "state.h"

namespace LightStateMachine
{
    typedef stlplus::digraph<State,bool> StateGraph;
}
