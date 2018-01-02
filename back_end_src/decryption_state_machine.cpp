#include "decryption_state_machine.h"
#include "state_graph.h"
#include "state_machine.h"
#include "emsg_exception.h"
#include "decryption_state_handlers.h"
#include "decryption_state_debug.h"

using namespace LightStateMachine;
using namespace LibEncryptMsg;

namespace EncryptPad
{

    int ID(StateID state_id)
    {
        return static_cast<StateMachineStateID>(state_id);
    }

    struct Graph
    {
        StateGraph state_graph;
        StateGraph::iterator start_node;
        StateGraph::iterator fail_node;
    };

    // The graph of states is built here.
    std::unique_ptr<Graph> BuildStateGraph()
    {
        std::unique_ptr<Graph> graph(new Graph());
        // Insert new state
        auto I = [&](State s)->StateGraph::iterator
        {
            return graph->state_graph.insert(s);
        };

        // Link states
        auto L = [&](StateGraph::iterator l, StateGraph::iterator r)
        {
            graph->state_graph.arc_insert(l, r);
        };

        // Stub functions
        auto T = [](StateMachineContext &){ return true; };
        auto F = [](StateMachineContext &){ return false; };
        auto Stub = [](StateMachineContext &){};

        // Start state
        auto start = I(State(ID(StateID::Start)));
        graph->start_node = start;

        // Fail state
        auto fail = I(State(ID(StateID::Fail), Fail_OnEnter, Stub, T, F));
        graph->fail_node = fail;

        auto read_in = I(State(ID(StateID::ReadIn), ReadIn_OnEnter, Stub,
                    ReadIn_CanEnter, T));
        auto parse_format = I(State(ID(StateID::ParseFormat), ParseFormat_OnEnter, Stub,
                    ParseFormat_CanEnter, T));
        auto set_pwd_key = I(State(ID(StateID::SetPwdKey), SetPwdKey_OnEnter, Stub,
                    SetPwdKey_CanEnter, T));
        auto read_key_file = I(State(ID(StateID::ReadKeyFile), ReadKeyFile_OnEnter, Stub,
                    ReadKeyFile_CanEnter, T));
        auto gpg = I(State(ID(StateID::GPG), GPG_OnEnter, Stub,
                    GPG_CanEnter, T));
        auto wad_head = I(State(ID(StateID::WADHead), WADHead_OnEnter, Stub,
                    WADHead_CanEnter, T));
        auto write_out = I(State(ID(StateID::WriteOut), WriteOut_OnEnter, Stub,
                    WriteOut_CanEnter, T));
        auto end = I(State(ID(StateID::End), End_OnEnter, Stub,
                    End_CanEnter, F));

        L(start, read_in);

        L(read_in, end);
        L(read_in, parse_format);
        L(read_in, gpg);
        L(read_in, wad_head);

        L(parse_format, read_in);
        L(parse_format, gpg);
        L(parse_format, wad_head);
        L(parse_format, read_key_file);
        L(parse_format, set_pwd_key);
        L(parse_format, write_out);

        L(set_pwd_key, gpg);

        L(read_key_file, read_in);
        L(read_key_file, gpg);

        L(gpg, read_in);
        L(gpg, parse_format);
        L(gpg, wad_head);
        L(gpg, gpg);
        L(gpg, write_out);

        L(wad_head, read_key_file);
        L(wad_head, gpg);
        L(wad_head, read_in);

        L(write_out, read_in);
        return graph;
    }

    class DecryptionStateIDToStringConverter: public LightStateMachine::StateIDToStringConverter
    {
        public:
            std::string Convert(StateMachineStateID state_id) override
            {
                return PrintDecryptionStateMachineStateID(state_id);
            }
    };
    LibEncryptMsg::PacketResult DecryptStream(InStream &in, const EncryptParams &encrypt_params,
            OutStream &out, PacketMetadata &metadata)
    {
        DecryptionContext context(in, out, encrypt_params, metadata);
        auto graph = BuildStateGraph();

        StateMachine state_machine(graph->state_graph, graph->start_node, graph->fail_node, context);
        state_machine.SetStateIDToStringConverter(
                std::unique_ptr<StateIDToStringConverter>(new DecryptionStateIDToStringConverter())
                );
        while(state_machine.NextState())
        {
        }

        return context.GetResult();
    }
}

