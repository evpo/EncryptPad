//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "packet_writer.h"
#include <memory>
#include "assert.h"
#include "botan/compression.h"
#include "botan/sha160.h"
#include "botan/cipher_mode.h"
#include "botan/auto_rng.h"
#include "openpgp_conversions.h"

using namespace std;
using SafeVector = Botan::secure_vector<uint8_t>;

namespace EncryptMsg
{
    void WriteLength(OutStream &stm, size_t len)
    {
        if(len < 192)
        {
            stm.Put(len);
        }
        else if(len < 8384)
        {
            len -= 192;
            stm.Put((len / 256) + 192); 
            stm.Put(len % 256);
        }
        else 
        {
            stm.Put(0xff);
            stm.Put((len >> 24)&0xff);
            stm.Put((len >> 16)&0xff);
            stm.Put((len >> 8)&0xff);
            stm.Put(len & 0xff);
        }
    }

    void WritePacketHeader(OutStream &out, const PacketHeader &header)
    {
        // Max length 0xffffffff - 0xffff - 1, which is about 4 gb
        assert(header.body_length < (0xffffffff - 0xffff - 1));
        uint8_t c = 0x80;
        if(header.is_new_format)
            c |= 0x40;

        uint8_t t = static_cast<uint8_t>(header.packet_type);
        c |= header.is_new_format ? t : t << 2;
        if(!header.is_new_format && header.is_partial_length)
        {
            c |= 0x03;
            out.Put(c);
        }
        else if(header.is_new_format && header.is_partial_length)
        {
            out.Put(c);
        }
        else if(header.is_new_format && !header.is_partial_length)
        {
            out.Put(c);
            WriteLength(out, header.body_length);
        }
        else
        {
            assert(false); // length type is not supported
        }
    }

    void WritePartialBlocks(InBufferStream &in, OutStream &out, uint8_t partial_length_power)
    {
        // partialBodyLen = 1 << (1st_octet & 0x1F)
        assert(partial_length_power == (partial_length_power & 0x1F));
        size_t length = 1 << partial_length_power;
        SafeVector buf(length);
        uint8_t header = partial_length_power | 0xE0;
        while(in.GetCount() >= length)
        {
            in.Read(buf.data(), buf.size());
            out.Put(header);
            out.Write(buf.data(), buf.size());
        }
    }

    void WriteSymmetricKeyESK(const MessageConfig &config, Salt salt, OutStream &out)
    {
        const size_t kESKPacketLength = 13;
        PacketHeader header;
        header.body_length = kESKPacketLength;
        header.packet_type = PacketType::SymmetricKeyESK;
        header.is_new_format = true;
        WritePacketHeader(out, header);
        out.Put(4);
        out.Put(static_cast<uint8_t>(config.GetCipherAlgo()));
        out.Put(3); //salt and iterations
        out.Put(static_cast<uint8_t>(config.GetHashAlgo()));
        out.Write(salt.data(), salt.size());
        out.Put(config.GetIterations());
    }

    class SymmetricIntegProtectedWriter : public PacketWriter
    {
        protected:
            void DoWrite(OutStream &out) override;
            void DoFinish(OutStream &out) override;
            void DoWriteHeader(OutStream &out) override;
        private:
            Botan::SHA_160 hash_;
            std::unique_ptr<Botan::Cipher_Mode> cipher_mode_;
            bool write_version_;
        public:
            SymmetricIntegProtectedWriter(const MessageConfig &config, Salt salt, const EncryptionKey &encryption_key);

    };

    class CompressedWriter : public PacketWriter
    {
        private:
            std::unique_ptr<Botan::Compression_Algorithm> compression_;
            bool write_compression_;
        protected:
            void DoWrite(OutStream &out) override;
            void DoFinish(OutStream &out) override;
            void DoWriteHeader(OutStream &out) override;
        public:
            CompressedWriter(const MessageConfig &config, Salt salt, const EncryptionKey &encryption_key);
    };

    class LiteralWriter : public PacketWriter
    {
        private:
            bool write_prefix_;
            void WritePrefix(OutStream &out);
        protected:
            void DoWrite(OutStream &out) override;
            void DoFinish(OutStream &out) override;
            void DoWriteHeader(OutStream &out) override;
        public:
            LiteralWriter(const MessageConfig &config, Salt salt, const EncryptionKey &encryption_key):PacketWriter(config, salt, encryption_key),
            write_prefix_(true){}
    };

    PacketWriter *PacketWriterFactory::GetOrCreate(PacketType packet_type, const MessageConfig &config, Salt salt, const EncryptionKey &encryption_key)
    {
        using PacketPtr = unique_ptr<PacketWriter>;
        auto it = packet_map_.find(packet_type);
        if(it == packet_map_.end())
        {
            PacketPtr packet_ptr;
            switch(packet_type)
            {
                case PacketType::SymmetricIntegProtected:
                    packet_ptr.reset(new SymmetricIntegProtectedWriter(config, salt, encryption_key));
                    break;
                case PacketType::Compressed:
                    packet_ptr.reset(new CompressedWriter(config, salt, encryption_key));
                    break;
                case PacketType::Literal:
                    packet_ptr.reset(new LiteralWriter(config, salt, encryption_key));
                    break;
                default:
                    // unknown packet type
                    assert(false);
                    break;
            }
            pair<const PacketType, PacketPtr> p(packet_type, std::move(packet_ptr));
            auto ret_pair = packet_map_.insert(move(p));
            it = ret_pair.first;
        }

        return it->second.get();
    }

    PacketWriter::PacketWriter(const MessageConfig &config, Salt salt, const EncryptionKey &encryption_key):
        config_(config), encryption_key_(encryption_key), salt_(salt),
        write_header_(true),
        finish_(false)
    {
    }

    void PacketWriter::WriteHeader(OutStream &out)
    {
        if(write_header_)
        {
            DoWriteHeader(out);
            write_header_ = false;
        }
    }

    void PacketWriter::Write(OutStream &out)
    {
        WriteHeader(out);
        SafeVector buf;
        auto buf_stm = MakeOutStream(buf);
        DoWrite(*buf_stm);
        out_.Push(buf);
        WritePartialBlocks(out_, out, config_.GetPartialLengthPower());
    }

    void PacketWriter::Finish(OutStream &out)
    {
        Write(out);
        SafeVector buf;
        auto buf_stm = MakeOutStream(buf);
        DoFinish(*buf_stm);
        out_.Push(buf);
        WritePartialBlocks(out_, out, config_.GetPartialLengthPower());
        // Write what's left
        if(out_.GetCount() == 0)
            return;
        buf.resize(out_.GetCount());
        out_.Read(buf.data(), buf.size());
        WriteLength(out, buf.size());
        out.Write(buf.data(), buf.size());
    }

    InBufferStream &PacketWriter::GetInStream()
    {
        return in_;
    }

    void LiteralWriter::DoWriteHeader(OutStream &out)
    {
        PacketHeader header;
        header.packet_type = PacketType::Literal;
        header.body_length = 0;
        header.is_new_format = true;
        header.is_partial_length = true;
        WritePacketHeader(out, header);
    }

    void LiteralWriter::WritePrefix(OutStream &out)
    {
        if(!write_prefix_)
            return;

        out.Put(config_.GetBinary() ? 'b' : 't');
        out.Put(static_cast<uint8_t>(config_.GetFileName().size()));
        out.Write(reinterpret_cast<const uint8_t*>(config_.GetFileName().data()),
                config_.GetFileName().size());
        auto file_date = config_.GetFileDate();
        out.Write(reinterpret_cast<const uint8_t*>(&file_date), 4);

        write_prefix_ = false;
    }

    void LiteralWriter::DoWrite(OutStream &out)
    {
        WritePrefix(out);
        SafeVector buf(in_.GetCount());
        in_.Read(buf.data(), buf.size());
        out.Write(buf.data(), buf.size());
    }

    void LiteralWriter::DoFinish(OutStream &out)
    {
        DoWrite(out);
    }

    CompressedWriter::CompressedWriter(const MessageConfig &config, Salt salt, const EncryptionKey &encryption_key)
        :PacketWriter(config, salt, encryption_key),
        write_compression_(true)
    {
        auto &compression_spec = GetCompressionSpec(config_.GetCompression());
        compression_.reset(Botan::make_compressor(compression_spec.botan_name));
        if(compression_)
            compression_->start();
    }

    void CompressedWriter::DoWriteHeader(OutStream &out)
    {
        PacketHeader header;
        header.packet_type = PacketType::Compressed;
        header.is_new_format = true;
        header.is_partial_length = true;
        WritePacketHeader(out, header);
    }

    void CompressedWriter::DoWrite(OutStream &out)
    {
        if(!compression_)
        {
            throw std::runtime_error("Compression is not initialised");
        }

        if(write_compression_)
        {
            out.Put(static_cast<uint8_t>(config_.GetCompression()));
            write_compression_ = false;
        }

        SafeVector buf(in_.GetCount());
        in_.Read(buf.data(), buf.size());

        if(finish_)
            compression_->finish(buf);
        else
            compression_->update(buf);

        out.Write(buf.data(), buf.size());

    }

    void CompressedWriter::DoFinish(OutStream &out)
    {
        finish_ = true;
        DoWrite(out);
    }

    SymmetricIntegProtectedWriter::SymmetricIntegProtectedWriter(const MessageConfig &config, Salt salt, const EncryptionKey &encryption_key)
        :PacketWriter(config, salt, encryption_key),
        write_version_(true)
    {
        auto &algo_spec = GetAlgoSpec(config_.GetCipherAlgo());
        cipher_mode_.reset(Botan::get_cipher_mode(algo_spec.botan_name, Botan::ENCRYPTION));
        cipher_mode_->set_key(encryption_key_.begin(), encryption_key_.size());
        SafeVector iv(algo_spec.block_size, 0);
        cipher_mode_->start(iv.data(), iv.size());

        SafeVector prefix;
        prefix.resize(algo_spec.block_size);
        Botan::AutoSeeded_RNG rng;
        rng.randomize(prefix.data(), prefix.size());
        prefix.push_back(*(prefix.begin() + prefix.size() - 2));
        prefix.push_back(*(prefix.begin() + prefix.size() - 2));
        in_.Push(prefix);
    }

    void SymmetricIntegProtectedWriter::DoWriteHeader(OutStream &out)
    {
        PacketHeader header;
        header.packet_type = PacketType::SymmetricIntegProtected;
        header.is_new_format = true;
        header.is_partial_length = true;
        WritePacketHeader(out, header);
    }

    void SymmetricIntegProtectedWriter::DoWrite(OutStream &out)
    {
        const size_t kMDCLength = 22;

        if(write_version_)
        {
            out.Put(1);// write packet version
            write_version_ = false;
        }

        size_t bytes2update = 0;
        if(!finish_)
        {
            bytes2update =
                in_.GetCount() / cipher_mode_->update_granularity() * cipher_mode_->update_granularity();
        }
        else
        {
            // size for finish
            bytes2update = in_.GetCount();
            assert(bytes2update + kMDCLength >= cipher_mode_->minimum_final_size());
        }

        SafeVector buf(bytes2update);
        in_.Read(buf.data(), bytes2update);
        hash_.update(buf.data(), bytes2update);

        if(!finish_)
        {
            cipher_mode_->update(buf);
        }
        else
        {
            hash_.update(0xD3);
            hash_.update(0x14);
            buf.push_back(0xD3);
            buf.push_back(0x14);

            SafeVector sha1 = hash_.final();
            buf.insert(buf.end(), sha1.begin(), sha1.begin() + sha1.size());
            assert(buf.size() == bytes2update + kMDCLength);
            cipher_mode_->finish(buf);
        }

        out.Write(buf.data(), buf.size());
    }

    void SymmetricIntegProtectedWriter::DoFinish(OutStream &out)
    {
        finish_ = true;
        DoWrite(out);
    }
}

