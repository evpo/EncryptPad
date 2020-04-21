//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "packet_reader.h"
#include <map>
#include <utility>
#include <algorithm>
#include "botan/cipher_mode.h"
#include "botan/compression.h"
#include "botan/hash.h"
#include "plog/Log.h"
#include "session_state.h"
#include "openpgp_conversions.h"
#include "message_encryption.h"
#include "emsg_constants.h"
#include "emsg_mem.h"

using namespace std;
using namespace EncryptMsg;

namespace
{
    EmsgResult CheckPrefix(const SafeVector &prefix)
    {
        // check the last two bytes for integrity of the message
        const uint8_t *ptr = prefix.data();
        ptr += prefix.size() - 4;
        if(*ptr != *(ptr + 2))
            return EmsgResult::InvalidSurrogateIV;
        if(*(ptr + 1) != *(ptr + 3))
            return EmsgResult::InvalidSurrogateIV;

        return EmsgResult::Success;
    }
}

namespace EncryptMsg
{
    class SymmetricKeyESKRW : public PacketRWBase
    {
    public:
        SymmetricKeyESKRW(SessionState &state)
            :PacketRWBase(state, true) // is_final_packet = true
        {
        }

    protected:
        EmsgResult DoRead(OutStream &out) override;
        EmsgResult DoFinish() override;
    };

    class SymmetricRWBase : public PacketRWBase
    {
        public:
            SymmetricRWBase(SessionState &state);

        protected:
            Botan::secure_vector<uint8_t> prefix_;
            std::unique_ptr<Botan::Cipher_Mode> cipher_mode_;
    };

    class SymmetricRW : public SymmetricRWBase
    {
        public:
            SymmetricRW(SessionState &state);
        protected:
            EmsgResult DoRead(OutStream &out) override;
            EmsgResult DoFinish() override;
        private:
            bool read_prefix_;
    };

    class SymmetricIntegProtectedRW : public SymmetricRWBase
    {
        public:
            SymmetricIntegProtectedRW(SessionState &state);

        protected:
            EmsgResult DoRead(OutStream &out) override;
            EmsgResult DoFinish() override;
        private:
            const size_t kMDCLength = 20 + 2;
            std::unique_ptr<Botan::HashFunction> hash_;
            bool version_read_;
            Botan::secure_vector<uint8_t> mdc_;
    };

    class CompressedRW : public PacketRWBase
    {
        public:
            CompressedRW(SessionState &state);

        protected:
            EmsgResult DoRead(OutStream &out) override;
            EmsgResult DoFinish() override;

        private:
            bool algo_read_;
            void ReadAlgo();
            std::unique_ptr<Botan::Decompression_Algorithm> decompression_;
    };

    PacketFactory::PacketFactory(SessionState &session_state):session_state_(session_state)
    {
    }

    EmsgResult HeaderReader::Read(bool finish_packets)
    {
        if(in_stm_.GetCount() < kHeaderMaxSize && !finish_packets)
            return EmsgResult::Pending;
        packet_header_ = ReadPacketHeader(in_stm_);
        if(GetPacketSpec(packet_header_.packet_type).packet_type == PacketType::Unknown)
            return EmsgResult::UnsupportedPacketType;

        return EmsgResult::Success;
    }

    class LiteralRW : public PacketRWBase
    {
        public:
            LiteralRW(SessionState &state);

        protected:
            EmsgResult DoRead(OutStream &out) override;
            EmsgResult DoFinish() override;
        private:
            bool header_read_;
            bool name_and_date_read_;
            uint8_t file_name_length_;
            EmsgResult ReadHeader();
            EmsgResult ReadNameAndDate();
    };

    std::pair<PacketRWBase*, bool> PacketFactory::GetOrCreatePacket(PacketType packet_type)
    {
        using PacketPtr = unique_ptr<PacketRWBase>;
        auto it = packet_map_.find(packet_type);
        bool new_packet = false;
        if(it == packet_map_.end())
        {
            PacketPtr packet_ptr;
            switch(packet_type)
            {
                case PacketType::SymmetricKeyESK:
                    packet_ptr.reset(new SymmetricKeyESKRW(session_state_));
                    break;
                case PacketType::SymmetricIntegProtected:
                    packet_ptr.reset(new SymmetricIntegProtectedRW(session_state_));
                    break;
                case PacketType::Symmetric:
                    packet_ptr.reset(new SymmetricRW(session_state_));
                    break;
                case PacketType::Compressed:
                    packet_ptr.reset(new CompressedRW(session_state_));
                    break;
                case PacketType::Literal:
                    packet_ptr.reset(new LiteralRW(session_state_));
                    break;
                default:
                    // unknown packet type
                    assert(false);
                    break;
            }
            pair<const PacketType, PacketPtr> p(packet_type, std::move(packet_ptr));
            auto ret_pair = packet_map_.insert(move(p));
            it = ret_pair.first;
            new_packet = true;
        }

        // the unique_ptr should not be null
        assert(it->second);
        return std::make_pair(it->second.get(), new_packet);
    }

    // Packet readers implementation

    SymmetricRWBase::SymmetricRWBase(SessionState &state)
                :PacketRWBase(state, false) // is_final_packet = false
    {
        bool cancelled = false;
        state.encryption_key = state.key_provider->GetKey(
                state_.message_config.GetCipherAlgo(),
                state_.message_config.GetHashAlgo(),
                state_.message_config.GetIterations(),
                state_.salt,
                "", cancelled);

        if(cancelled || !state.encryption_key)
        {
            state.emsg_result = EmsgResult::InvalidPassphrase;
            return;
        }

        auto algo_spec = GetAlgoSpec(state_.message_config.GetCipherAlgo());
        cipher_mode_.reset(Botan::get_cipher_mode(algo_spec.botan_name, Botan::DECRYPTION));
        auto &key = state_.encryption_key;
        vector<uint8_t> iv(algo_spec.block_size, 0);
        cipher_mode_->set_key(key->begin(), key->size());
        cipher_mode_->start(iv);
    }

    SymmetricRW::SymmetricRW(SessionState &state)
        : SymmetricRWBase(state), read_prefix_(true){}

    SymmetricIntegProtectedRW::SymmetricIntegProtectedRW(SessionState &state)
        : SymmetricRWBase(state),
        hash_(Botan::HashFunction::create_or_throw("SHA-160")),
        version_read_(false)
    {}

    EmsgResult SymmetricRW::DoRead(OutStream &out)
    {
        if(state_.emsg_result == EmsgResult::InvalidPassphrase)
        {
            state_.is_message_analyzed = true;
            return state_.emsg_result;
        }

        auto &algo_spec = GetAlgoSpec(state_.message_config.GetCipherAlgo());

        if(read_prefix_)
        {
            const unsigned prefix_len = algo_spec.block_size + 2;
            if(in_.GetCount() < prefix_len)
                return EmsgResult::Pending;

            SafeVector enc_prefix(prefix_len);
            in_.Read(enc_prefix.data(), enc_prefix.size());
            SafeVector prefix(enc_prefix);
            this->cipher_mode_->finish(prefix);
            EmsgResult result = CheckPrefix(prefix);
            if(result != EmsgResult::Success)
                return result;

            // resync the cipher
            enc_prefix.erase(enc_prefix.begin(), enc_prefix.begin() + 2);
            cipher_mode_->start(enc_prefix.data(), enc_prefix.size());
            read_prefix_ = false;
        }

        size_t bytes2update = 0;

        if(!state_.finish_packets)
        {
            bytes2update =
                in_.GetCount() / cipher_mode_->update_granularity() * cipher_mode_->update_granularity();
        }
        else
        {
            // size for finish
            bytes2update = in_.GetCount();
            assert(bytes2update >= cipher_mode_->minimum_final_size());
        }

        Botan::secure_vector<uint8_t> buf(bytes2update);
        in_.Read(buf.data(), buf.size());

        if(state_.finish_packets)
        {
            cipher_mode_->finish(buf);
        }
        else if(buf.size() > 0)
        {
            cipher_mode_->update(buf);
        }

        if(buf.size() == 0)
            return EmsgResult::Pending;

        out.Write(buf.data(), buf.size());

        // We expect Finish to return Success
        return EmsgResult::Pending;
    }

    EmsgResult SymmetricRW::DoFinish()
    {
        return EmsgResult::Success;
    }

    EmsgResult SymmetricIntegProtectedRW::DoRead(OutStream &out)
    {
        if(state_.emsg_result == EmsgResult::InvalidPassphrase)
        {
            state_.is_message_analyzed = true;
            return state_.emsg_result;
        }

        auto &algo_spec = GetAlgoSpec(state_.message_config.GetCipherAlgo());
        if(!version_read_)
        {
            uint8_t b = in_.Get();
            version_read_ = true;
            if(b != 1)
                return EmsgResult::UnexpectedFormat;
        }

        const size_t prefix_len = algo_spec.block_size + 2;

        // Run until the MDC tag starts. 20 is hash length and 2 for the tag and the length

        size_t bytes2update = 0;

        if(!state_.finish_packets)
        {
            bytes2update =
                in_.GetCount() / cipher_mode_->update_granularity() * cipher_mode_->update_granularity();

            if(bytes2update < prefix_len + kMDCLength)
                bytes2update = 0;
        }
        else
        {
            // size for finish
            bytes2update = in_.GetCount();
            assert(bytes2update >= cipher_mode_->minimum_final_size());
        }


        Botan::secure_vector<uint8_t> buf(bytes2update);
        in_.Read(buf.data(), buf.size());

        if(state_.finish_packets)
        {
            cipher_mode_->finish(buf);
        }
        else if(buf.size() > 0)
        {
            cipher_mode_->update(buf);
        }

        if(buf.size() == 0)
            return EmsgResult::Pending;

        if(prefix_.size() < prefix_len)
        {
            size_t bytes4prefix = std::min(prefix_len - prefix_.size(), buf.size());
            prefix_.resize(prefix_.size() + bytes4prefix);
            std::copy_n(buf.data(), bytes4prefix,
                    prefix_.data() + prefix_.size() - bytes4prefix);
            buf.erase(buf.begin(), buf.begin() + bytes4prefix);
            if(prefix_.size() == prefix_len)
            {
                auto result = CheckPrefix(prefix_);
                if(result != EmsgResult::Success)
                    return result;
                hash_->update(prefix_);
            }
        }

        assert(mdc_.size() == 0 || mdc_.size() == kMDCLength);

        if(buf.size() >= kMDCLength)
        {
            if(mdc_.size() == kMDCLength) // not first iteration
            {
                // previous left bytes
                hash_->update(mdc_.data(), kMDCLength);
                out.Write(mdc_.data(), kMDCLength);
            }
            else
            {
                mdc_.resize(kMDCLength);
            }

            hash_->update(buf.data(), buf.size() - kMDCLength);
            out.Write(buf.data(), buf.size() - kMDCLength);

            // new left bytes
            std::copy_n(buf.begin() + buf.size() - kMDCLength,
                    kMDCLength, mdc_.begin());
        }
        else
        {
            assert(state_.finish_packets);
            hash_->update(mdc_.data(), buf.size());
            out.Write(mdc_.data(), buf.size());

            // this is a move to the left. Hopefully it handles overlapping ranges.
            std::copy_n(mdc_.begin() + buf.size(), kMDCLength - buf.size(), mdc_.begin());
            std::copy_n(buf.begin(), buf.size(), mdc_.begin() + kMDCLength - buf.size());
        }

        assert(mdc_.size() == kMDCLength);

        // We expect Finish to return Success
        return EmsgResult::Pending;
    }

    EmsgResult SymmetricIntegProtectedRW::DoFinish()
    {
        using namespace Botan;

        InBufferStream stm;
        stm.Push(mdc_);
        if(stm.Get() != 0xD3)
            return EmsgResult::UnexpectedFormat;

        if(stm.Get() != 0x14)
            return EmsgResult::UnexpectedFormat;

        hash_->update(0xD3);
        hash_->update(0x14);

        secure_vector<uint8_t> actual_sha1 = hash_->final();
        secure_vector<uint8_t> expected_sha1;
        expected_sha1.resize(20);
        stm.Read(expected_sha1.data(), 20);
        assert(stm.GetCount() == 0);

        if(actual_sha1 != expected_sha1)
            return EmsgResult::MDCError;

        return EmsgResult::Success;
    }

    EmsgResult SymmetricKeyESKRW::DoRead(OutStream &out)
    {
        const size_t kRequiredLength = 13;

        (void)out;

        if(in_.GetCount() < kRequiredLength)
            return EmsgResult::Pending;

        if(in_.Get()!=4)
            return EmsgResult::UnexpectedFormat;

        state_.message_config.SetCipherAlgo(static_cast<CipherAlgo>(in_.Get()));
        switch(state_.message_config.GetCipherAlgo())
        {
            case CipherAlgo::TripleDES:
            case CipherAlgo::CAST5:
            case CipherAlgo::AES128:
            case CipherAlgo::AES192:
            case CipherAlgo::AES256:
            case CipherAlgo::Twofish:
                break;
            default:
                return EmsgResult::UnsupportedAlgo;
        }

        if(in_.Get() != 3)
            return EmsgResult::UnsupportedS2K;

        state_.message_config.SetHashAlgo(static_cast<HashAlgo>(in_.Get()));
        switch(state_.message_config.GetHashAlgo())
        {
            case HashAlgo::SHA160:
            case HashAlgo::SHA256:
            case HashAlgo::SHA384:
            case HashAlgo::SHA512:
            case HashAlgo::SHA224:
                break;
            default:
                return EmsgResult::UnsupportedS2K;
        }

        auto &salt = state_.salt;
        salt.resize(kSaltSize);
        if(in_.Read(salt.data(), salt.size()) != static_cast<size_t>(salt.size()))
            return EmsgResult::UnexpectedFormat;

        state_.message_config.SetIterations(in_.Get());
        // return empty because this packet doesn't have children
        return EmsgResult::Success;
    }

    EmsgResult SymmetricKeyESKRW::DoFinish()
    {
        return EmsgResult::Success;
    }

    CompressedRW::CompressedRW(SessionState &state)
        : PacketRWBase(state, false), algo_read_(false)
    {
    }

    void CompressedRW::ReadAlgo()
    {
        if(algo_read_)
            return;
        uint8_t b = in_.Get();
        algo_read_ = true;
        state_.message_config.SetCompression(static_cast<Compression>(b));
        //TODO: move the validation to SetCompression
        switch(state_.message_config.GetCompression())
        {
            case Compression::ZIP:
            case Compression::ZLIB:
            case Compression::BZip2:
                break;
            default:
                state_.emsg_result = EmsgResult::UnsupportedCompressionAlgo;
                return;
        }
        auto &compression_spec = GetCompressionSpec(state_.message_config.GetCompression());
        decompression_.reset(Botan::make_decompressor(compression_spec.botan_name));
        if(decompression_)
            decompression_->start();
    }

    EmsgResult CompressedRW::DoRead(OutStream &out)
    {
        ReadAlgo();
        if(!decompression_)
            return state_.emsg_result;

        Botan::secure_vector<uint8_t> buf(in_.GetCount());
        in_.Read(buf.data(), buf.size());

        if(state_.finish_packets)
            decompression_->finish(buf);
        else
            decompression_->update(buf);

        out.Write(buf.data(), buf.size());

        return EmsgResult::Pending;
    }

    EmsgResult CompressedRW::DoFinish()
    {
        return EmsgResult::Success;
    }

    LiteralRW::LiteralRW(SessionState &state)
        : PacketRWBase(state, true), // true - final packet
        header_read_(false), name_and_date_read_(false), file_name_length_(0)
    {
    }

    EmsgResult LiteralRW::ReadHeader()
    {
        if(header_read_)
            return EmsgResult::Success;

        const size_t kHeaderLength = 2;
        if(in_.GetCount() < kHeaderLength)
            return EmsgResult::Pending;

        uint8_t b = in_.Get();
        switch(b)
        {
            // b binary, t text, u UTF8, l local, 1 local too
            case 'b': case 't': case 'u': case 'l': case '1':
                break;
            default:
                return EmsgResult::UnexpectedFormat;
        }

        // it is the only important thing for us for now
        state_.message_config.SetBinary(b == 'b');
        file_name_length_ = in_.Get();
        header_read_ = true;
        return EmsgResult::Success;
    }

    EmsgResult LiteralRW::ReadNameAndDate()
    {
        if(name_and_date_read_)
            return EmsgResult::Success;

        const size_t required_length = 4 + file_name_length_; // date is 4 bytes
        if(in_.GetCount() < required_length)
            return EmsgResult::Pending;

        Botan::secure_vector<uint8_t> buffer;
        buffer.resize(file_name_length_ + 1);// 1 for the terminator
        in_.Read(buffer.data(), file_name_length_);
        *(buffer.data() + buffer.size() - 1) = '\0';

        state_.message_config.SetFileName(reinterpret_cast<const char*>(buffer.data()));

        buffer.resize(4);
        in_.Read(buffer.data(), buffer.size());
        FileDate file_date = state_.message_config.GetFileDate();
        file_date = 0;

        static_assert(sizeof(file_date) >= 4, "file_date type must be 4 bytes or more");
        std::copy_n(buffer.data(), 4, reinterpret_cast<uint8_t*>(&file_date));
        state_.message_config.SetFileDate(file_date);
        state_.is_message_analyzed = true;
        name_and_date_read_ = true;
        return EmsgResult::Success;
    }

    EmsgResult LiteralRW::DoRead(OutStream &out)
    {
        EmsgResult result = ReadHeader();
        if(result != EmsgResult::Success)
            return result;

        result = ReadNameAndDate();
        if(result != EmsgResult::Success)
            return result;

        Botan::secure_vector<uint8_t> buffer(in_.GetCount());
        in_.Read(buffer.data(), buffer.size());
        out.Write(buffer.data(), buffer.size());

        return EmsgResult::Pending;
    }

    EmsgResult LiteralRW::DoFinish()
    {
        return EmsgResult::Success;
    }

}
