#pragma once
#include "state_graph.h"

namespace LibEncryptMsg
{
    struct StateGraphInfo
    {
        LightStateMachine::StateGraph *state_graph;
        LightStateMachine::StateGraph::iterator start_node;
        LightStateMachine::StateGraph::iterator fail_node;
    };

    // The graph of states is built here.
    StateGraphInfo BuildStateGraph();
}

