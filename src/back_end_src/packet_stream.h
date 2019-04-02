//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//This file is part of EncryptPad
//
//EncryptPad is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 2 of the License, or
//(at your option) any later version.
//
//EncryptPad is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EncryptPad.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************

#pragma once
#include <iterator>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <queue>
#include <stdexcept>
#include "assert.h"
#include "packet_typedef.h"
#include "epad_utilities.h"

namespace EncryptPad
{
    typedef unsigned char byte;

    class OutStream : public NonCopyableNonMovable
    {
    protected:
        stream_length_type count_;

        OutStream():count_(0) {};
        virtual bool DoPut(byte b) = 0;
        virtual bool DoWrite(const byte *in_it, stream_length_type bytes2write) = 0;
    public:
        void SetCount(stream_length_type count)
        {
            count_ = count;
        }

        stream_length_type GetCount() const
        {
            return count_;
        }

        bool Put(byte b)
        {
            return DoPut(b);
        }

        bool Write(const byte *in_it, stream_length_type bytes2write)
        {
            return DoWrite(in_it, bytes2write);
        }

        virtual ~OutStream()
        {
        }
    };

    class OutPacketStreamCont : public OutStream
    {
    private:
        Botan::SecureVector<byte> *out_;
        Botan::SecureVector<byte>::iterator ptr_;

        void Resize(size_t size)
        {
            auto offset = ptr_ - out_->begin();
            size_t new_size = std::max(size, out_->size() * 2);
            out_->resize(new_size);
            ptr_ = out_->begin() + offset;
        }
    protected:
        virtual bool DoPut(byte b) override;

        virtual bool DoWrite(const byte *in_it, stream_length_type bytes2write) override;
    public:
        OutPacketStreamCont():out_{} {}
        OutPacketStreamCont(Botan::SecureVector<byte> &out):out_(&out), ptr_(out.begin()) {}

        void Reset()
        {
            if(!out_)
                return;

            ptr_ = out_->begin();
            count_ = 0;
        }

        void Set(Botan::SecureVector<byte> &out)
        {
            out_ = &out;
            ptr_ = out.begin();
            count_ = 0;
        }
    };

    std::unique_ptr<OutPacketStreamCont> MakeOutStream(Botan::SecureVector<byte> &cnt);

    class OutPacketStreamFile : public OutStream
    {
    private:
        FileHndl file_;
    protected:
        virtual bool DoPut(byte b);
        virtual bool DoWrite(const byte *in_it, stream_length_type bytes2write);

    public:
        OutPacketStreamFile():file_() {}
        OutPacketStreamFile(FileHndl file):file_(file) {}

        void Set(FileHndl file)
        {
            file_ = file;
            count_ = 0;
        }

    };

    //Start Stream Interfaces
    class InStream;

    struct EOFHandlerBase
    {
    public:
        virtual void operator()(InStream &stm) = 0;
        virtual ~EOFHandlerBase()
        {
        }
    };

    // This function reads from the source, updates all counters and calls EOFHandler.
    // It accesses ReadFromSource method that should not be public. 
    // For that reason the function is a friend of two strem types below.
    stream_length_type ReadWithEOFHandler(InStream &stm, byte *out_it, stream_length_type bytes2read);

    class InStream : public NonCopyableNonMovable
    {
    protected:
        virtual bool DoIsEOF() const = 0;
        virtual int DoGet() = 0;
        virtual stream_length_type DoRead(byte *out_it, stream_length_type bytes2read) = 0;
        virtual stream_length_type DoGetCount() const = 0;
        virtual void DoSetCount(stream_length_type count) = 0;
        virtual EOFHandlerBase *DoGetEOFHandler() = 0;
        virtual void DoSetEOFHandler(EOFHandlerBase *eof_handler) = 0;
        // Reads from the source directly without updating counters. It is called internally from DoRead call stack.
        virtual stream_length_type ReadFromSource(byte *out_it, stream_length_type length) = 0;
    public:
        stream_length_type GetCount() const
        {
            return DoGetCount();
        }

        void SetCount(stream_length_type count)
        {
            DoSetCount(count);
        }

        void SetEOFHandler(EOFHandlerBase *eof_handler)
        {
            DoSetEOFHandler(eof_handler);
        }

        EOFHandlerBase *GetEOFHandler()
        {
            return DoGetEOFHandler();
        }

        bool IsEOF() const
        {
            return DoIsEOF();
        }

        // Check for -1 to see if the stream is depleted
        int Get()
        {
            return DoGet();
        }

        // Reads bytes2read bytes to out_it iterator
        stream_length_type Read(byte *out_it, stream_length_type bytes2read)
        {
            return DoRead(out_it, bytes2read);
        }

        virtual ~InStream() {}

        friend stream_length_type ReadWithEOFHandler(InStream &stm, byte *out_it, stream_length_type bytes2read);
    };

    class RandomInStream : public InStream
    {
    protected:
        virtual bool DoSeek(stream_length_type offset) = 0;
    public:
        bool Seek(stream_length_type offset)
        {
            return DoSeek(offset);
        }
    };
    // End Stream Interfaces


    class InPacketStreamFile : public RandomInStream
    {
    private:
        FileHndl file_;
        stream_length_type length_;
        stream_length_type count_;
        EOFHandlerBase *eof_handler_;
    protected:

        virtual stream_length_type DoGetCount() const override
        {
            return count_;
        }

        virtual void DoSetCount(stream_length_type count) override
        {
            count_ = count;
        }

        virtual void DoSetEOFHandler(EOFHandlerBase *eof_handler) override
        {
            eof_handler_ = eof_handler;
        }

        virtual EOFHandlerBase *DoGetEOFHandler() override
        {
            return eof_handler_;
        }

        virtual bool DoIsEOF() const override
        {
            return !count_ || feof(file_.get()) || ferror(file_.get());
        }

        virtual int DoGet() override
        {
            if(!count_ && eof_handler_)
            {
                (*eof_handler_)(*this);
            }

            if(!count_ || feof(file_.get()))
                return -1;

            count_--;
            return fgetc(file_.get());
        }

        virtual stream_length_type ReadFromSource(byte *out_it, stream_length_type length) override
        {
			stream_length_type ret_val = fread(out_it, sizeof(byte), length, file_.get());
			return ret_val;
        }

        virtual stream_length_type DoRead(byte *out_it, stream_length_type bytes2read) override
        {
            return ReadWithEOFHandler(*this, out_it, bytes2read);
        }

        virtual bool DoSeek(stream_length_type offset) override
        {
            //eof_handler_ can't be used with Seek
            assert(!eof_handler_);
            if(eof_handler_)
                return false;
            count_ = length_ - offset;
            return !fseek(file_.get(), offset, SEEK_SET);
        }

    public:
        InPacketStreamFile():
            file_(), length_(0), count_(0), eof_handler_(nullptr) {}

        InPacketStreamFile(FileHndl file, stream_length_type length):
            file_(file), length_(length), count_(length_), eof_handler_(nullptr) 
        {
            assert(length >= 0);
        }

        void Set(FileHndl file, stream_length_type length)
        {
            if(file_)
                fclose(file_.get());

            file_ = file;
            count_ = length;
            length_ = length;
            assert(length >= 0);
        }
    };

    class InPacketStreamMemory : public RandomInStream
    {
    private:
        const byte *begin_;
        const byte *end_;
        const byte *pos_;
        stream_length_type count_;
        EOFHandlerBase *eof_handler_;
    protected:

        virtual stream_length_type DoGetCount() const override
        {
            return count_;
        }

        virtual void DoSetCount(stream_length_type count) override
        {
            count_ = count;
        }

        virtual void DoSetEOFHandler(EOFHandlerBase *eof_handler) override
        {
            eof_handler_ = eof_handler;
        }

        virtual EOFHandlerBase *DoGetEOFHandler() override
        {
            return eof_handler_;
        }

        virtual bool DoIsEOF() const override
        {
            return (!count_ || pos_ == end_);
        }

        virtual int DoGet() override
        {
            if(!count_ && eof_handler_)
            {
                (*eof_handler_)(*this);
            }

            if(!count_ || pos_ == end_)
                return -1;

            count_--;
            int ret_val = *pos_;
            ++pos_;
            return ret_val;
        }

        virtual stream_length_type ReadFromSource(byte *out_it, stream_length_type length) override
        {
            stream_length_type bytes2read = std::min(length, static_cast<stream_length_type>(end_ - pos_));
            std::copy_n(pos_, bytes2read, out_it);
            pos_ += bytes2read;
            return bytes2read;
        }

        virtual stream_length_type DoRead(byte *out_it, stream_length_type bytes2read) override
        {
            return ReadWithEOFHandler(*this, out_it, bytes2read);
        }

        virtual bool DoSeek(stream_length_type offset) override
        {
            assert(!GetEOFHandler());
            if(GetEOFHandler())
                return false;

            const byte *new_pos = begin_ + offset;
            if(new_pos >= end_)
                return false;

            pos_ = new_pos;
            count_ = end_ - pos_;
            return true;
        }
    public:
        InPacketStreamMemory():begin_{}, end_{}, pos_{}, count_(0), eof_handler_(nullptr) {}
        InPacketStreamMemory(const byte *b, const byte *e)
            : begin_(b), end_(e), pos_(b), count_(e - b), eof_handler_(nullptr) 
        {
            assert(e >= b);
        }

        void Set(const byte *b, const byte *e)
        {
            pos_ = b;
            end_ = e;
            count_ = e - b;
            assert(count_ >= 0);
        }

    };

    class InPacketStreamPipe : public InStream
    {
    private:
        stream_length_type count_;
        std::queue<InStream*> streams_;
    protected:
        virtual bool DoIsEOF() const override
        {
            return count_ == 0 || streams_.empty();
        }

        virtual int DoGet() override
        {
            int b = -1;

            if(count_ == 0)
                return b;

            while(!streams_.empty())
            {
                b = streams_.front()->Get();
                if(b != -1)
                {
                    count_--;
                    assert(count_ >= 0);
                    break;
                }

                streams_.pop();
            }

            return b;
        }

        virtual stream_length_type DoRead(byte *out_it, stream_length_type bytes2read) override;

        virtual stream_length_type DoGetCount() const override
        {
            return count_;
        }

        virtual void DoSetCount(stream_length_type count) override
        {
            count_ = count;
        }

        virtual EOFHandlerBase *DoGetEOFHandler() override
        {
            return nullptr;
        }

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
        virtual void DoSetEOFHandler(EOFHandlerBase *eof_handler) override
        {
        }

        virtual stream_length_type ReadFromSource(byte *out_it, stream_length_type bytes2read) override
        {
            return 0;
        }
#pragma GCC diagnostic pop

    public:
        void Push(InStream &stm)
        {
            streams_.push(&stm);
            count_ += stm.GetCount();
        }

        void Clear()
        {
            while(!streams_.empty())
                streams_.pop();

            count_ = 0;
        }

        InPacketStreamPipe()
            :count_(0) 
        {
        }

        virtual ~InPacketStreamPipe() {}

    };
}
