#include "armor_writer.h"
#include <string>
#include "assert.h"
#include "botan/hash.h"
#include "botan/base64.h"
#include "plog/Log.h"

namespace
{
    static const std::string kHeader = "-----BEGIN PGP MESSAGE-----";
    static const std::string kFooter = "-----END PGP MESSAGE-----";
}

namespace EncryptMsg
{
    ArmorWriter::ArmorWriter():
        part_length_(0),
        line_slots_(kArmorLineLength)
    {

    }

    void ArmorWriter::Start()
    {
        crc24_ = Botan::HashFunction::create_or_throw("CRC24");
        status_ = ArmorWriterStatus::Header;
    }

    void ArmorWriter::Write(OutStream &out, bool finish)
    {
        using namespace Botan;

        if(status_ == ArmorWriterStatus::Disabled || status_ == ArmorWriterStatus::Finished)
            throw std::runtime_error("ArmorWriter needs to be started");

        bool process = (finish || in_.GetCount() > 0);
        while(process)
        {
            switch(status_)
            {
                case ArmorWriterStatus::Header:
                    out.Write(reinterpret_cast<const uint8_t*>(&kHeader.front()), kHeader.size());
                    out.Put('\n');
                    out.Put('\n');

                    status_ = ArmorWriterStatus::Payload;

                    // special case: zero length input data
                    // jump to footer
                    if(finish && in_.GetCount() == 0)
                    {
                        status_ = ArmorWriterStatus::Footer;
                    }
                    break;
                case ArmorWriterStatus::Payload:
                    {
                        if(finish && in_.GetCount() == 0 && part_length_ == 0)
                        {
                            status_ = ArmorWriterStatus::Footer;
                            break;
                        }

                        buffer_in_.resize(in_.GetCount());
                        in_.Read(buffer_in_.data(), buffer_in_.size());

                        if(part_length_ > 0)
                        {
                            buffer_in_.insert(buffer_in_.begin(),
                                    part_.begin(), part_.begin() + part_length_);
                            part_length_ = 0;
                        }

                        buffer_out_.resize(base64_encode_max_output(buffer_in_.size()));
                        size_t consumed = 0;
                        size_t result = base64_encode(
                                reinterpret_cast<char*>(buffer_out_.data()), buffer_in_.data(),
                                buffer_in_.size(), consumed,
                                finish);
                        crc24_->update(buffer_in_.data(), consumed);
                        buffer_out_.resize(result);
                        LOG_INFO << "buffer_in_.size() : " << buffer_in_.size() << " consumed : " << consumed;
                        part_length_ = buffer_in_.size() - consumed;
                        assert(part_length_ == 0 || !finish);
                        std::copy(buffer_in_.begin() + consumed, buffer_in_.end(), part_.begin());
                        FormatLines(buffer_out_, out);
                        if(finish)
                            status_ = ArmorWriterStatus::Footer;
                    }
                    break;
                case ArmorWriterStatus::Footer:
                    {
                        auto crc = crc24_->final();
                        std::string crc_str = base64_encode(crc.data(), crc.size());
                        out.Put('\n');
                        out.Put('=');
                        out.Write(reinterpret_cast<const uint8_t*>(crc_str.data()), crc_str.size());
                        out.Put('\n');
                        out.Write(reinterpret_cast<const uint8_t*>(kFooter.data()), kFooter.size());
                        out.Put('\n');
                        status_ = ArmorWriterStatus::Finished;
                    }
                    break;
                default:
                    assert(false);
            }
            process = (in_.GetCount() > 0 || status_ == ArmorWriterStatus::Footer);
        }
    }

    void ArmorWriter::FormatLines(const std::vector<uint8_t> &buf, OutStream &out)
    {
        auto it = buf.begin();
        unsigned int dist = std::min(
                line_slots_,
                static_cast<unsigned int>(std::distance(it, buf.end()))
                );

        while(it != buf.end())
        {
            out.Write(&(*it), dist);

            it += dist;

            assert(dist <= line_slots_);
            line_slots_ -= dist;
            if(line_slots_ == 0)
            {
                line_slots_ = kArmorLineLength;
                out.Put('\n');
            }

            dist = std::min(
                    line_slots_,
                    static_cast<unsigned int>(std::distance(it, buf.end()))
                    );
        }
    }

    InBufferStream &ArmorWriter::GetInStream()
    {
        return in_;
    }

    ArmorWriterStatus ArmorWriter::GetStatus() const
    {
        return status_;
    }

    ArmorWriter::~ArmorWriter()
    {
    }
}
