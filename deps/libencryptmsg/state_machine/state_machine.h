#pragma once

#include <queue>
#include "state_machine_interface.h"
#include "state_graph.h"
#include "state_machine_utility.h"

namespace LightStateMachine
{
    class StateMachine : private NonCopyable
    {
        public:
            StateMachine(StateGraph &state_graph, StateGraph::iterator start_state, StateGraph::iterator fail_state, StateMachineContext &context);
            bool NextState();
            StateMachineStateID CurrentState() const;
            StateMachineStateID PreviousState() const;
            void Reset();
        private:
            void SetCurrentState(StateGraph::iterator new_state);
            StateGraph *state_graph_;
            StateGraph::iterator start_state_;
            StateGraph::iterator current_state_;
            StateGraph::iterator fail_state_;
            StateMachineContext *context_;
            std::queue<StateMachineStateID> state_queue_;
    };
}
