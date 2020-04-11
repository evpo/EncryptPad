#pragma once
#include "memory_stream.h"
#include "emsg_types.h"
#include "packet_reader.h"
#include <vector>
#include <string>
#include <memory>

namespace EncryptMsg
{
    enum class ArmorStatus
    {
        Unknown,
        Disabled,
        Enabled,
    };

    struct ArmorReaderImpl;
    class ArmorReader final : public NonCopyableNonMovable
    {
        private:
            std::unique_ptr<ArmorReaderImpl> pimpl_;
        public:
            ArmorReader();
            ~ArmorReader();
            EmsgResult Read(OutStream &out);
            EmsgResult Finish(OutStream &out);
            InBufferStream &GetInStream();
            ArmorStatus GetStatus() const;
    };
}
