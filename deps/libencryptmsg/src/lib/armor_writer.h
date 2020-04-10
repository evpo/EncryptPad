#pragma once
#include <memory>
#include <array>
#include "memory_stream.h"
#include "utility.h"

namespace Botan
{
    class HashFunction;
}

namespace EncryptMsg
{
    enum class ArmorWriterStatus
    {
        Disabled,
        Header,
        Payload,
        Footer,
        Finished,
    };

    static const unsigned int kArmorBlockLength = 3;
    static const unsigned int kArmorLineLength = 64;

    class ArmorWriter : public NonCopyableNonMovable
    {
        public:
            ArmorWriter();
            ~ArmorWriter();
            void Start();
            void Write(OutStream &out, bool finish);
            InBufferStream &GetInStream();
            ArmorWriterStatus GetStatus() const;
        private:

            InBufferStream in_;
            std::vector<uint8_t> buffer_in_;
            std::vector<uint8_t> buffer_out_;
            std::array<uint8_t, kArmorBlockLength> part_;
            unsigned int part_length_;
            ArmorWriterStatus status_ = ArmorWriterStatus::Disabled;
            std::unique_ptr<Botan::HashFunction> crc24_;
            unsigned int line_slots_;

            void FormatLines(const std::vector<uint8_t> &buf, OutStream &out);
    };
}
