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
            SecureVector buffer_;
            SecureVector length_buffer_;
            uint8_t *begin_;
            uint8_t *end_;
            uint8_t *pos_;
            bool partial_length_;

            size_t GetPartialCount() const;

            void SetPartialCount(size_t count);

            SecureVector::const_iterator ReadLength(SecureVector::const_iterator it, SecureVector::const_iterator end);

            // Reads from the source directly without updating counters.
            size_t ReadFromSource(uint8_t *out_it, size_t length);
        public:
            InBufferStream();

            bool GetPartialLength() const;

            // Set partial length flag and set the length if it's already read
            void SetPartialLength(bool flag, size_t length = 0);

            size_t GetCount() const;

            void Push(SecureVector &buf);

            bool IsEOF() const;

            // Check for -1 to see if the stream is depleted
            int Get();

            // Reads bytes2read bytes to out_it iterator
            size_t Read(uint8_t *out_it, size_t bytes2read);
    };

    class OutStream : public NonCopyableNonMovable
    {
    private:
        SecureVector &out_;
        uint8_t *ptr_;
        void Resize(size_t size);
    public:
        OutStream(SecureVector &out);
        void Reset();
        bool Put(uint8_t b);
        bool Write(const uint8_t *in_it, size_t bytes2write);
    };

    std::unique_ptr<OutStream> MakeOutStream(SecureVector &cnt);

    void AppendToBuffer(InBufferStream &stm, SecureVector &buf);
}
