//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include "emsg_mem.h"
#include "emsg_utility.h"

namespace LibEncryptMsg
{
    class InBufferStream;

    class InBufferStream : public NonCopyableNonMovable
    {
        private:
            size_t count_;
            SafeVector buffer_;
            SafeVector length_buffer_;
            uint8_t *begin_;
            uint8_t *end_;
            uint8_t *pos_;
            bool partial_length_;

            size_t GetPartialCount() const;

            void SetPartialCount(size_t count);

            SafeVector::const_iterator ReadLength(SafeVector::const_iterator it, SafeVector::const_iterator end);

            // Reads from the source directly without updating counters.
            size_t ReadFromSource(uint8_t *out_it, size_t length);
        public:
            InBufferStream();

            bool GetPartialLength() const;

            // Set partial length flag and set the length if it's already read
            void SetPartialLength(bool flag, size_t length = 0);

            size_t GetCount() const;

            void Push(SafeVector &buf);

            bool IsEOF() const;

            // Check for -1 to see if the stream is depleted
            int Get();

            // Reads bytes2read bytes to out_it iterator
            size_t Read(uint8_t *out_it, size_t bytes2read);
    };

    class OutStream : public NonCopyableNonMovable
    {
    private:
        SafeVector &out_;
        uint8_t *ptr_;
        void Resize(size_t size);
    public:
        OutStream(SafeVector &out);
        void Reset();
        bool Put(uint8_t b);
        bool Write(const uint8_t *in_it, size_t bytes2write);
    };

    std::unique_ptr<OutStream> MakeOutStream(SafeVector &cnt);

    void AppendToBuffer(InBufferStream &stm, SafeVector &buf);
}
