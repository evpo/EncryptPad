//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "state_machine.h"
#include <vector>
#include <string>
#include <memory>
#include <string>

#ifdef TRACE_STATE_MACHINE
#include "plog/Log.h"
#endif

using namespace std;

namespace
{
#ifdef TRACE_STATE_MACHINE
    void DebugPrintState(std::string str)
    {
        LOG_DEBUG << str;
    }
#endif
}


namespace LightStateMachine
{

    unsigned kQueueSize = 3U;

    std::string StandardStateIDToStringConverter(StateMachineStateID state_id);

    std::string StandardStateIDToStringConverter(StateMachineStateID state_id)
    {
        return std::string("default : state_id = ") + std::to_string(state_id);
    }

    StateMachine::StateMachine(StateGraph &state_graph, StateMachineContext &context)
        :state_graph_(&state_graph),
        is_first_entry_(true),
        context_(&context)
    {
    }

    void StateMachine::SetStateIDToStringConverter(StateIDToStringConverter converter)
    {
        state_id_to_string_converter_ = converter;
    }

    void StateMachine::Reset()
    {
        context_->SetFailed(false);
        is_first_entry_ = true;
    }

    bool StateMachine::NextState()
    {
        if(is_first_entry_)
        {
            start_state_ = state_graph_->StatesMap()[state_graph_->GetStartStateID()];
            current_state_ = start_state_;
            fail_state_ = state_graph_->StatesMap()[state_graph_->GetFailStateID()];
            is_first_entry_ = false;
        }

        if(!current_state_->CanExit(*context_))
            return false;

        bool state_changed = false;
        for(unsigned i = 0U; i < state_graph_->GetGraph().fanout(current_state_); i ++)
        {
            auto arc_it = state_graph_->GetGraph().output(current_state_, i);
            auto it = state_graph_->GetGraph().arc_to(arc_it);
            context_->SetIsReentry(current_state_->GetID() == it->GetID());
            if(it->CanEnter(*context_))
            {
                current_state_->OnExit(*context_);
                SetCurrentState(it);
                current_state_->OnEnter(*context_);
                state_changed = !context_->GetFailed();

                break;
            }
        }

        context_->SetIsReentry(false);
        if(state_changed)
            return true;

        // check if we are already in fail_state. It should not be reentrant
        if(current_state_->GetID() == fail_state_->GetID())
        {
            return false;
        }

        if(fail_state_->CanEnter(*context_))
        {
            SetCurrentState(fail_state_);
            current_state_->OnEnter(*context_);
        }

        return true;
    }

    StateMachineStateID StateMachine::CurrentState() const
    {
        return current_state_->GetID();
    }

    void StateMachine::SetCurrentState(DigraphIt new_state)
    {
        state_queue_.push(current_state_->GetID());
        while(state_queue_.size() > kQueueSize)
        {
            state_queue_.pop();
        }
        current_state_ = new_state;
#ifdef TRACE_STATE_MACHINE
        DebugPrintState(state_id_to_string_converter_(current_state_->GetID()));
#endif
    }

    StateMachineStateID StateMachine::PreviousState() const
    {
        return state_queue_.back();
    }
}
