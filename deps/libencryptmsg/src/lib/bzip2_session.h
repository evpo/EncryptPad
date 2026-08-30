//**********************************************************************************
//LibEncryptMsg Copyright 2026 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <cstddef>
#include "bzlib.h"
#include "emsg_mem.h"
#include "memory_stream.h"

namespace EncryptMsg
{

enum class Bzip2SessionResult
{
    Ok,
    OutputBufferOverflow,
    StreamEnd,
    Error,
};

class Bzip2Session
{
private:
    static const size_t kStreamBufferSize = 64<<10; //64 Kb
    size_t output_buffer_size_;
    bz_stream bz_stm_;
    SafeVector bz_in_;
    SafeVector bz_out_;
    SafeVector rejected_input_;
    bool init_failed_;
    bool stream_end_;

public:

    Bzip2Session(size_t output_buffer_size);
    ~Bzip2Session();
    bool InitFailed() const;
    Bzip2SessionResult Read(InBufferStream &in, OutStream &out);
};

class CompressBzip2Session
{
private:
    static const size_t kStreamBufferSize = 64<<10; //64 Kb
    bz_stream bz_stm_;
    SafeVector bz_in_;
    SafeVector bz_out_;
    bool init_failed_;

public:
    CompressBzip2Session();
    ~CompressBzip2Session();
    bool InitFailed() const;
    Bzip2SessionResult Read(InBufferStream &in, OutStream &out, bool finish);
};

}
