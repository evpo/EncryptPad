#include "armor_reader.h"
#include <vector>
#include <map>
#include <algorithm>
#include "botan/hash.h"
#include "botan/base64.h"
#include "plog/Log.h"
#include "session_state.h"
#include "assert.h"
#include "state_machine.h"

using namespace LightStateMachine;

namespace
{
    bool is_space(char c)
    {
        return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
    }
    static const unsigned int kMaxLineLength = 76;
    static const unsigned int kArmorBlockSize = 4;
    static const std::string kMinHeader = "-----BEGIN PGP ";
    static const std::string kMinTail = "-----END PGP ";
    static const std::string kSeparator = "-----";
}

namespace EncryptMsg
{
    enum class ArmorState : StateMachineStateID
    {
        Start,
        Unknown,
        BeginHeader,
        Header,
        Payload,
        CRC,
        Tail,
        TailFound,
        Disabled,
        Fail,
    };

    std::string GetStateName(StateMachineStateID state_id);
    std::string GetStateName(StateMachineStateID state_id)
    {
        static const std::vector<std::string> names = 
        {
            "Start",
            "Unknown",
            "BeginHeader",
            "Header",
            "Payload",
            "CRC",
            "Tail",
            "TailFound",
            "Disabled",
            "Fail",
        };
        const std::string &name = names[static_cast<size_t>(state_id)];
        return std::string("armor : ") + name;
    }

    enum class Result
    {
        None,
        Pending,
        Disabled,
        UnexpectedFormat,
        Success,
    };

    struct LineResult
    {
        bool success = false;
        bool max_length_reached = false;
        std::vector<uint8_t> line;
    };

    struct ArmorReaderImpl
    {
        std::string label_;
        InBufferStream in_stm_;
        SafeVector buffer_;
        std::unique_ptr<Botan::HashFunction> crc24_;
        bool finish_;
        Result result_;
        ArmorStatus status_;

        StateMachineContext context_;
        StateGraph state_graph_;
        StateMachine state_machine_;

        OutStream *out_;

        bool ValidateCRC(const std::string &crc);
        LineResult NextLine();
        Result ReadUnknown();
        void Unknown_OnEnter(StateMachineContext& context);

        Result ReadBeginHeader();
        Result ReadHeader();
        Result ReadPayload(OutStream &out);
        Result ReadCRC();
        Result ReadTail();

        bool Shared_CanEnter(StateMachineContext& context);
        bool IsNotPendingOrBuffer(StateMachineContext& context);
        bool Unknown_CanEnter(StateMachineContext& context);
        bool Disabled_CanEnter(StateMachineContext& context);
        void Disabled_OnEnter(StateMachineContext& context);
        void BeginHeader_OnEnter(StateMachineContext& context);
        void Header_OnEnter(StateMachineContext& context);
        void Payload_OnEnter(StateMachineContext& context);
        void CRC_OnEnter(StateMachineContext& context);
        void Tail_OnEnter(StateMachineContext& context);
        void TailFound_OnEnter(StateMachineContext& context);
        void Fail_OnEnter(StateMachineContext& context);

        ArmorReaderImpl();

        EmsgResult Read(OutStream &out);
        EmsgResult Finish(OutStream &out);
    };

    InBufferStream &ArmorReader::GetInStream()
    {
        return pimpl_->in_stm_;
    }

    ArmorStatus ArmorReader::GetStatus() const
    {
        return pimpl_->status_;
    }

    EmsgResult ArmorReader::Read(OutStream &out)
    {
        return pimpl_->Read(out);
    }
    EmsgResult ArmorReader::Finish(OutStream &out)
    {
        return pimpl_->Finish(out);
    }

    ArmorReader::ArmorReader():
        pimpl_(new ArmorReaderImpl())
    {
    }

    ArmorReader::~ArmorReader()
    {
    }

    ArmorReaderImpl::ArmorReaderImpl():
        crc24_(Botan::HashFunction::create_or_throw("CRC24")),
        finish_(false),
        result_(Result::None),
        status_(ArmorStatus::Unknown),
        state_machine_(state_graph_, context_),
        out_(nullptr)
    {
        using VoidMember = VoidMemberFunction<ArmorReaderImpl>;
        using BoolMember = BoolMemberFunction<ArmorReaderImpl>;
        using Self = ArmorReaderImpl;
        state_machine_.SetStateIDToStringConverter(GetStateName);
        auto &sg = state_graph_;

        sg.Create(ArmorState::Start);
        sg.Create(ArmorState::Fail, VoidMember(this, &ArmorReaderImpl::Fail_OnEnter));
        sg.SetStartStateID(ArmorState::Start);
        sg.SetFailStateID(ArmorState::Fail);

        sg.Create(ArmorState::Unknown,
                VoidMember(this, &Self::Unknown_OnEnter), StubVoidFunction,
                BoolMember(this, &Self::Unknown_CanEnter), BoolMember(this, &Self::IsNotPendingOrBuffer));

        sg.Create(ArmorState::BeginHeader,
                VoidMember(this, &Self::BeginHeader_OnEnter), StubVoidFunction,
                BoolMember(this, &Self::Shared_CanEnter), BoolMember(this, &Self::IsNotPendingOrBuffer));

        sg.Create(ArmorState::Header,
                VoidMember(this, &Self::Header_OnEnter), StubVoidFunction,
                BoolMember(this, &Self::Shared_CanEnter), BoolMember(this, &Self::IsNotPendingOrBuffer));

        sg.Create(ArmorState::Payload,
                VoidMember(this, &Self::Payload_OnEnter), StubVoidFunction,
                BoolMember(this, &Self::Shared_CanEnter), BoolMember(this, &Self::IsNotPendingOrBuffer));

        sg.Create(ArmorState::CRC,
                VoidMember(this, &Self::CRC_OnEnter), StubVoidFunction,
                BoolMember(this, &Self::Shared_CanEnter), BoolMember(this, &Self::IsNotPendingOrBuffer));

        sg.Create(ArmorState::Tail,
                VoidMember(this, &Self::Tail_OnEnter), StubVoidFunction,
                BoolMember(this, &Self::Shared_CanEnter), BoolMember(this, &Self::IsNotPendingOrBuffer));

        sg.Create(ArmorState::TailFound,
                VoidMember(this, &Self::TailFound_OnEnter), StubVoidFunction,
                BoolMember(this, &Self::Shared_CanEnter), BoolMember(this, &Self::IsNotPendingOrBuffer));

        sg.Create(ArmorState::Disabled,
                VoidMember(this, &Self::Disabled_OnEnter), StubVoidFunction,
                BoolMember(this, &Self::Disabled_CanEnter), BoolMember(this, &Self::IsNotPendingOrBuffer));

        sg.Get(ArmorState::Disabled).SetCanExit(AlwaysFalseBoolFunction);
        sg.Get(ArmorState::TailFound).SetCanExit(AlwaysFalseBoolFunction);

        sg.Link(ArmorState::Start, ArmorState::Unknown);

        sg.Link(ArmorState::Unknown, ArmorState::Unknown);
        sg.Link(ArmorState::Unknown, ArmorState::Disabled);
        sg.Link(ArmorState::Unknown, ArmorState::BeginHeader);

        sg.Link(ArmorState::BeginHeader, ArmorState::BeginHeader);
        sg.Link(ArmorState::BeginHeader, ArmorState::Header);

        sg.Link(ArmorState::Header, ArmorState::Header);
        sg.Link(ArmorState::Header, ArmorState::Payload);

        sg.Link(ArmorState::Payload, ArmorState::Payload);
        sg.Link(ArmorState::Payload, ArmorState::CRC);

        sg.Link(ArmorState::CRC, ArmorState::Tail);

        sg.Link(ArmorState::Tail, ArmorState::Tail);
        sg.Link(ArmorState::Tail, ArmorState::TailFound);
    }

    Result ArmorReaderImpl::ReadUnknown()
    {
        // when we finish, we return the data to in_stm_
        // in other states we keep the data in buffer_

        PushBackToBuffer(in_stm_, buffer_);
        if(buffer_.size() < kMinHeader.size())
        {
            if(!finish_)
                return Result::Pending;
            else
            {
                in_stm_.Push(buffer_);
                return Result::Disabled;
            }
        }

        result_ = std::equal(kMinHeader.begin(), kMinHeader.end(), buffer_.begin()) ?
            Result::Success : Result::Disabled;

        in_stm_.Push(buffer_);
        buffer_.clear();
        return result_;
    }

    Result ArmorReaderImpl::ReadBeginHeader()
    {
        auto line_result = NextLine();
        if(line_result.max_length_reached)
            return Result::UnexpectedFormat;
        if(!line_result.success)
            return Result::Pending;

        auto &line = line_result.line;
        assert(line.size() >= kMinHeader.size());
        int label_size = static_cast<int>(line.size()) - kMinHeader.size() - kSeparator.size();
        if(label_size < 0 || label_size > static_cast<int>(kMaxLineLength))
            return Result::UnexpectedFormat;

        auto label_it = line.begin() + kMinHeader.size();
        label_.assign(label_it, label_it + label_size);
        LOG_INFO << "label : " << label_;
        return std::equal(kSeparator.begin(), kSeparator.end(), label_it + label_size) ?
            Result::Success : Result::UnexpectedFormat;
    }

    Result ArmorReaderImpl::ReadHeader()
    {
        auto line_result = NextLine();
        while(line_result.success)
        {
            if(std::all_of(line_result.line.begin(), line_result.line.end(), is_space))
                return Result::Success;
            line_result = NextLine();
        }

        if(line_result.max_length_reached)
        {
            LOG_WARNING << "UnexpectedFormat: armor line is too long";
            return Result::UnexpectedFormat;
        }

        return Result::Pending;
    }

    Result ArmorReaderImpl::ReadPayload(OutStream &out)
    {
        using namespace Botan;
        auto line_result = NextLine();
        while(line_result.success)
        {
            auto &line = line_result.line;
            if(line.size() > 0 && line[0] == '=')
            {
                line.push_back('\n');
                buffer_.insert(
                        buffer_.begin(),
                        line.begin(),
                        line.end());
                return Result::Success;
            }
            else if(line.size() % 4 != 0)
            {
                return Result::UnexpectedFormat;
            }
            else
            {
                std::vector<uint8_t> buf(base64_decode_max_output(line.size()), 0);
                size_t decode_res = base64_decode(buf.data(),
                        reinterpret_cast<char*>(line.data()), line.size(), false);
                buf.resize(decode_res);
                crc24_->update(buf.data(), buf.size());
                out.Write(buf.data(), buf.size());
            }
            line_result = NextLine();
        }

        if(line_result.max_length_reached)
        {
            LOG_WARNING << "UnexpectedFormat: armor line is too long";
            return Result::UnexpectedFormat;
        }

        return Result::Pending;
    }

    Result ArmorReaderImpl::ReadCRC()
    {
        using namespace Botan;
        auto line_result = NextLine();
        if(line_result.max_length_reached)
        {
            LOG_WARNING << "UnexpectedFormat: armor line is too long";
            return Result::UnexpectedFormat;
        }

        if(!line_result.success)
            return Result::Pending;

        if(line_result.line.size() != kArmorBlockSize + 1 || line_result.line[0] != '=')
            return Result::UnexpectedFormat;
        auto it = line_result.line.begin();
        it ++;
        std::string crc_str(it, line_result.line.end());
        auto is_valid = ValidateCRC(crc_str);
        if(!is_valid)
        {
            LOG_WARNING << "CRC is invalid";
            return Result::UnexpectedFormat;
        }
        return Result::Success;
    }

    Result ArmorReaderImpl::ReadTail()
    {
        auto line_result = NextLine();
        if(line_result.max_length_reached)
        {
            LOG_WARNING << "UnexpectedFormat: armor line is too long";
            return Result::UnexpectedFormat;
        }
        if(!line_result.success)
            return Result::Pending;

        std::string tail = kMinTail + label_ + kSeparator;
        bool valid = (line_result.line.size() == tail.size());
        valid = valid && std::equal(line_result.line.begin(), line_result.line.end(),
                tail.begin());

        if(!valid)
        {
            LOG_WARNING << "UnexpectedFormat: tail is invalid";
            return Result::UnexpectedFormat;
        }

        return Result::Success;
    }

    bool ArmorReaderImpl::ValidateCRC(const std::string &crc)
    {
        using namespace Botan;
        auto computed_crc = crc24_->final();
        std::string computed_crc_str(base64_encode_max_output(computed_crc.size()), 0);
        size_t consumed = 0;
        size_t written = base64_encode(&computed_crc_str.front(), computed_crc.data(),
                computed_crc.size(), consumed, true);
        assert(written == computed_crc_str.size());
        assert(crc.size() > 0);
        return crc == computed_crc_str;
    }

    LineResult ArmorReaderImpl::NextLine()
    {
        LineResult result;
        PushBackToBuffer(in_stm_, buffer_);
        auto it = std::find_if(buffer_.begin(), buffer_.end(),
               [](const uint8_t &b){return b == '\r' || b == '\n';});

        auto line_range_end = it;
        if(it != buffer_.end() && *it == '\r')
            it++;

        if(it == buffer_.end())
        {
            if(buffer_.size() > kMaxLineLength)
                result.max_length_reached = true;
            return result;
        }

        result.success = true;

        result.line.assign(buffer_.begin(), line_range_end);
        it ++;
        buffer_.erase(buffer_.begin(), it);
        return result;
    }

    bool ArmorReaderImpl::Shared_CanEnter(StateMachineContext& context)
    {
        if(result_ == Result::Pending && context.IsReentry())
            return true;

        if(result_ == Result::Success && !context.IsReentry())
            return true;

        return false;
    }

    bool ArmorReaderImpl::IsNotPendingOrBuffer(StateMachineContext&)
    {
        return (result_ != Result::Pending || in_stm_.GetCount() > 0);
    }

    // Unknown

    bool ArmorReaderImpl::Unknown_CanEnter(StateMachineContext&)
    {
        switch(result_)
        {
            case Result::None:
            case Result::Pending:
                return true;
            default:
                return false;
        }
    }

    void ArmorReaderImpl::Unknown_OnEnter(StateMachineContext&)
    {
        result_ = ReadUnknown();
    }

    // Disabled

    bool ArmorReaderImpl::Disabled_CanEnter(StateMachineContext&)
    {
        return (result_ == Result::Disabled);
    }

    void ArmorReaderImpl::Disabled_OnEnter(StateMachineContext&)
    {
        status_ = ArmorStatus::Disabled;
    }

    // BeginHeader

    void ArmorReaderImpl::BeginHeader_OnEnter(StateMachineContext&)
    {
        status_ = ArmorStatus::Enabled;
        result_ = ReadBeginHeader();
    }

    // Header

    void ArmorReaderImpl::Header_OnEnter(StateMachineContext&)
    {
        result_ = ReadHeader();
    }

    void ArmorReaderImpl::Payload_OnEnter(StateMachineContext&)
    {
        result_ = ReadPayload(*out_);
    }

    void ArmorReaderImpl::CRC_OnEnter(StateMachineContext&)
    {
        result_ = ReadCRC();
    }

    void ArmorReaderImpl::Tail_OnEnter(StateMachineContext&)
    {
        result_ = ReadTail();
    }

    void ArmorReaderImpl::TailFound_OnEnter(StateMachineContext&)
    {
    }

    void ArmorReaderImpl::Fail_OnEnter(StateMachineContext& context)
    {
        context.SetFailed(true);
    }

    EmsgResult ArmorReaderImpl::Read(OutStream &out)
    {
        if(result_ == Result::Success)
            return EmsgResult::Success;

        out_ = &out;

        while(state_machine_.NextState())
        {
        }

        if(context_.GetFailed())
        {
            return EmsgResult::UnexpectedError;
        }

        switch(result_)
        {
            case Result::Success:
            case Result::Disabled:
                return EmsgResult::Success;
            case Result::UnexpectedFormat:
                return EmsgResult::UnexpectedFormat;
            case Result::Pending:
                return EmsgResult::Pending;
            default:
                assert(false);
                return EmsgResult::UnexpectedError;
        }
    }

    EmsgResult ArmorReaderImpl::Finish(OutStream &out)
    {
        out_ = &out;
        finish_ = true;
        return Read(out);
    }
}
