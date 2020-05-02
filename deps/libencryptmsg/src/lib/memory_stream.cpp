//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "memory_stream.h"
#include <algorithm>
#include "assert.h"
#include "packet_parsers.h"
#include "emsg_constants.h"
#include "plog/Log.h"

namespace EncryptMsg
{
    std::unique_ptr<OutStream> MakeOutStream(Botan::secure_vector<uint8_t> &cnt)
    {
        std::unique_ptr<OutStream> ret_val(new OutStream(cnt));
        return ret_val;
    }

    bool OutStream::Put(uint8_t b)
    {
        assert(ptr_ <= out_.data() + out_.size());
        if(ptr_ == out_.data() + out_.size())
        {
            Resize(out_.size() + 1);
        }
        *ptr_++ = b;
        return true;
    }

    bool OutStream::Write(const uint8_t *in_it, size_t bytes2write)
    {
        assert(ptr_ <= out_.data() + out_.size());

        if(!bytes2write)
            return true;

        size_t free_bytes = out_.data() + out_.size() - ptr_;
        if(free_bytes < bytes2write)
        {
            Resize(out_.size() + bytes2write - free_bytes);
        }

        std::copy_n(in_it, bytes2write, ptr_);
        ptr_ += bytes2write;

        return true;
    }

    void OutStream::Resize(size_t size)
    {
        auto offset = ptr_ - out_.data();

        if(size > out_.capacity())
            out_.reserve(std::max(size, out_.capacity() * 2));

        out_.resize(size);
        ptr_ = out_.data() + offset;
    }

    OutStream::OutStream(Botan::secure_vector<uint8_t> &out):
        out_(out), ptr_(out.data())
    {
    }

    void OutStream::Reset()
    {
        out_.clear();
        ptr_ = out_.data();
    }

    void AppendToBuffer(InBufferStream &stm, Botan::secure_vector<uint8_t> &buf)
    {
        buf.resize(buf.size() + stm.GetCount());
        std::copy_n(buf.begin(), buf.size() - stm.GetCount(), buf.begin() + stm.GetCount());
        stm.Read(buf.data(), stm.GetCount());
    }

    void PushBackToBuffer(InBufferStream &stm, SafeVector &buf)
    {
        size_t prev_count = buf.size();
        buf.resize(prev_count + stm.GetCount());
        stm.Read(buf.data() + prev_count, stm.GetCount());
    }

    SafeVector::const_iterator InBufferStream::ReadLength(SafeVector::const_iterator it, SafeVector::const_iterator end)
    {
        size_t range_distance = static_cast<size_t>(std::distance(it, end));
        size_t bytes2take = std::max(range_distance, static_cast<size_t>(5U));

        SafeVector buf(it, it + bytes2take);
        InBufferStream stm;
        stm.Push(buf);

        // TODO: Add ReadLength overload to read from iterators so we don't need to make a temp buffer
        bool is_partial = false;
        SetPartialCount(EncryptMsg::ReadLength(stm, is_partial));
        if(!is_partial)
            SetPartialLength(false);
        return it + bytes2take - stm.GetCount();
    }

    size_t InBufferStream::GetPartialCount() const
    {
        return count_;
    }

    void InBufferStream::SetPartialCount(size_t count)
    {
        count_ = count;
    }

    // Reads from the source directly without updating counters.
    size_t InBufferStream::ReadFromSource(uint8_t *out_it, size_t length)
    {
        size_t bytes2read = std::min(length, static_cast<size_t>(end_ - pos_));
        std::copy_n(pos_, bytes2read, out_it);
        pos_ += bytes2read;
        return bytes2read;
    }

    InBufferStream::InBufferStream()
        :count_(kMaxStreamLength),
        begin_(nullptr), end_(nullptr), pos_(nullptr),
        partial_length_(false)
    {
        begin_ = pos_ = end_ = buffer_.data();
    }

    bool InBufferStream::GetPartialLength() const
    {
        return partial_length_;
    }

    // Set partial length flag and set the length if it's already read
    void InBufferStream::SetPartialLength(bool flag, size_t length)
    {
        partial_length_ = flag;

        assert(partial_length_ || !length);

        if(partial_length_)
        {
            SetPartialCount(length);
        }
    }

    size_t InBufferStream::GetCount() const
    {
        return end_ - pos_;
    }

    void InBufferStream::Push(Botan::secure_vector<uint8_t> &buf)
    {
        // erase everything before pos_ and append the content of buf
        buffer_.erase(buffer_.begin(), buffer_.begin() + (pos_ - begin_));
        auto it = buf.cbegin();
        while(it != buf.cend())
        {
            size_t bytes2take = std::min(static_cast<size_t>(buf.cend() - it), GetPartialCount());
            buffer_.insert(buffer_.end(), it, it + bytes2take);

            if(GetPartialLength())
                SetPartialCount(GetPartialCount() - bytes2take);

            it += bytes2take;
            if(!GetPartialCount() && GetPartialLength())
            {
                // length_buffer_ should be at least 5 bytes, which is max length
                // wait until it is filled before reading the length
                // after reading the length return the remaining bytes to the buffer and clear length_buffer_
                size_t bytes2fill_length_buf = std::min(static_cast<size_t>(buf.cend() - it), 5U - length_buffer_.size());
                length_buffer_.insert(length_buffer_.end(), it, it + bytes2fill_length_buf);
                it += bytes2fill_length_buf;
                if(length_buffer_.size() == 5U ||
                        (length_buffer_.size() >= 1 && TryOneOctetLength(length_buffer_[0]).is_one_octet_length))
                {
                    auto lb_it = length_buffer_.cbegin();
                    lb_it = ReadLength(lb_it, length_buffer_.cend());
                    buffer_.insert(buffer_.end(), lb_it, length_buffer_.cend());
                    SetPartialCount(GetPartialCount() - static_cast<size_t>(length_buffer_.cend() - lb_it)); // reduce the length for the remaining in the length buffer bytes
                    length_buffer_.clear();
                }
            }

            assert(GetPartialCount() || it == buf.cend() || bytes2take);
        }

        pos_ = begin_ = buffer_.data();
        end_ = buffer_.data() + buffer_.size();
    }

    bool InBufferStream::IsEOF() const
    {
        return GetCount() == 0U;
    }

    // Check for -1 to see if the stream is depleted
    int InBufferStream::Get()
    {
        if(pos_ == end_)
            return -1;

        int ret_val = *pos_;
        ++pos_;
        return ret_val;
    }

    // Reads bytes2read bytes to out_it iterator
    size_t InBufferStream::Read(uint8_t *out_it, size_t bytes2read)
    {
        int bytes_read = 0;

        while(bytes2read && !IsEOF())
        {
            size_t step_length = std::min(GetCount(), bytes2read);
            size_t bytes = ReadFromSource(out_it, step_length);
            bytes2read -= bytes;
            bytes_read += bytes;
            out_it += bytes;
        }

        return bytes_read;
    }
}
