#include "bzip2_session.h"
#include <algorithm>
#include <bzlib.h>
#include <iterator>
#include "plog/Log.h"

namespace
{
char *ToChar(uint8_t *p)
{
    return reinterpret_cast<char*>(p);
}
}

namespace EncryptMsg
{

Bzip2Session::Bzip2Session(size_t output_buffer_size):
    output_buffer_size_(output_buffer_size),
    init_failed_(false),
    stream_end_(false)
{
    bz_stm_.bzalloc = NULL;
    bz_stm_.bzfree = NULL;
    bz_stm_.opaque = NULL;
    bz_stm_.next_in = nullptr;
    bz_stm_.avail_in = 0;
    bz_stm_.next_out = nullptr;
    bz_stm_.avail_out = 0;

    const int verbosity = 0;
    int res = BZ2_bzDecompressInit(&bz_stm_, verbosity, 0);
    if(res != BZ_OK)
    {
        LOG_ERROR << "BZ2_bzDecompressInit failed. res: " << res;
        init_failed_ = true;
    }
}

Bzip2Session::~Bzip2Session()
{
    BZ2_bzDecompressEnd(&bz_stm_);
}

bool Bzip2Session::InitFailed() const
{
    return init_failed_;
}

Bzip2SessionResult Bzip2Session::Read(InBufferStream &in, OutStream &out)
{
    if(stream_end_)
    {
        if(in.GetCount() > 0)
        {
            LOG_ERROR << "Unexpected input after stream end. Count: " << in.GetCount();
            return Bzip2SessionResult::Error;
        }

        return Bzip2SessionResult::StreamEnd;
    }
    bz_in_.resize(rejected_input_.size() + in.GetCount());
    std::copy(rejected_input_.begin(), rejected_input_.end(), bz_in_.begin());
    in.Read(bz_in_.data() + rejected_input_.size(), in.GetCount());
    rejected_input_.clear();

    bz_stm_.next_in = ToChar(bz_in_.data());
    bz_stm_.avail_in = bz_in_.size();

    int res = 0;
    unsigned total_out_n = 0;
    unsigned out_n = 0;
    do
    {
        bz_out_.resize(Bzip2Session::kStreamBufferSize);
        bz_stm_.next_out = ToChar(bz_out_.data());
        bz_stm_.avail_out = bz_out_.size();
        res = BZ2_bzDecompress(&bz_stm_);
        out_n = std::distance(ToChar(bz_out_.data()), bz_stm_.next_out);
        total_out_n += out_n;
        LOG_DEBUG << "res: " << res << " ao:" << bz_stm_.avail_out << " to:" << bz_stm_.total_out_lo32
            << " ai:" << bz_stm_.avail_in << " ti:" << bz_stm_.total_in_lo32;

        switch(res)
        {
            case BZ_OK: case BZ_STREAM_END:
                break;
            default:
                LOG_ERROR << "Error: BZ2_bzDecompress res: " << res;
                return Bzip2SessionResult::Error;
        }

        out.Write(bz_out_.data(), out_n);
    }
    while(res == BZ_OK && out_n > 0 && total_out_n <= output_buffer_size_);

    bool output_buffer_overflow = total_out_n > output_buffer_size_;
    LOG_DEBUG << "output_buffer_overflow: " << output_buffer_overflow << " total_out_n: " << total_out_n;
    if(output_buffer_overflow)
    {
        rejected_input_.resize(bz_stm_.avail_in);
        std::copy_n(bz_stm_.next_in, bz_stm_.avail_in, ToChar(rejected_input_.data()));
        bz_in_.clear();
    }
    if(res == BZ_STREAM_END)
    {
        stream_end_ = true;
        return Bzip2SessionResult::StreamEnd;
    }
    return !output_buffer_overflow ? Bzip2SessionResult::Ok : Bzip2SessionResult::OutputBufferOverflow;
}


CompressBzip2Session::CompressBzip2Session():
    init_failed_(false)
{
    bz_stm_.bzalloc = NULL;
    bz_stm_.bzfree = NULL;
    bz_stm_.opaque = NULL;
    bz_stm_.next_in = nullptr;
    bz_stm_.avail_in = 0;
    bz_stm_.next_out = nullptr;
    bz_stm_.avail_out = 0;
    const int verbosity = 0;
    int res = BZ2_bzCompressInit(&bz_stm_, 5, verbosity, 0);
    if(res != BZ_OK)
    {
        LOG_ERROR << "BZ2_bzCompressInit failed. res: " << res;
        init_failed_ = true;
    }
}

bool CompressBzip2Session::InitFailed() const
{
    return init_failed_;
}

Bzip2SessionResult CompressBzip2Session::Read(InBufferStream &in, OutStream &out, bool finish)
{
    bz_in_.resize(in.GetCount());
    in.Read(bz_in_.data(), bz_in_.size());
    bz_stm_.next_in = ToChar(bz_in_.data());
    bz_stm_.avail_in = bz_in_.size();
    bz_stm_.next_out = ToChar(bz_out_.data());
    bz_stm_.avail_out = bz_out_.size();

    while(bz_stm_.avail_in > 0)
    {
        int res = BZ2_bzCompress(&bz_stm_, BZ_RUN);
        LOG_DEBUG << "res: " << res << " ao:" << bz_stm_.avail_out << " to:" << bz_stm_.total_out_lo32
            << " ai:" << bz_stm_.avail_in << " ti:" << bz_stm_.total_in_lo32;
        if(res != BZ_RUN_OK)
        {
            LOG_ERROR << "BZ2_bzCompress failed. res: " << res;
            return Bzip2SessionResult::Error;
        }
        out.Write(bz_out_.data(), std::distance(ToChar(bz_out_.data()), bz_stm_.next_out));
        bz_out_.resize(CompressBzip2Session::kStreamBufferSize);
        bz_stm_.next_out = ToChar(bz_out_.data());
        bz_stm_.avail_out = bz_out_.size();
    }

    if(!finish)
        return Bzip2SessionResult::Ok;

    int res = 0;
    do
    {
        res = BZ2_bzCompress(&bz_stm_, BZ_FINISH);
        switch(res)
        {
            case BZ_FINISH_OK: case BZ_STREAM_END:
                break;
            default:
                LOG_ERROR << "BZ2_bzCompress failed. res: " << res;
                return Bzip2SessionResult::Error;
        }
        out.Write(bz_out_.data(), std::distance(ToChar(bz_out_.data()), bz_stm_.next_out));
        bz_out_.resize(CompressBzip2Session::kStreamBufferSize);
        bz_stm_.next_out = ToChar(bz_out_.data());
        bz_stm_.avail_out = bz_out_.size();
    }
    while (res == BZ_FINISH_OK);

    return Bzip2SessionResult::Ok;
}

CompressBzip2Session::~CompressBzip2Session()
{
    BZ2_bzCompressEnd(&bz_stm_);
}


}
