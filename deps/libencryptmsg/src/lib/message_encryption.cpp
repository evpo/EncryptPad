//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "message_encryption.h"
#include <memory>
#include <array>
#include "assert.h"
#include "botan/pbkdf.h"
#include "context.h"
#include "packet_writer.h"
#include "session_state.h"
#include "algo_spec.h"
#include "openpgp_conversions.h"
#include "emsg_symmetric_key.h"
#include "emsg_constants.h"
#include "armor_writer.h"

using namespace std;
using namespace LightStateMachine;
using namespace EncryptMsg;

namespace EncryptMsg
{
    class MessageWriterImpl
    {
        private:
            std::unique_ptr<EncryptionKey> encryption_key_;
            MessageConfig message_config_;
            Salt salt_;
            std::array<PacketType, kMaxPacketChainLength> packet_chain_;
            PacketWriterFactory packet_writer_factory_;
            bool write_esk_;
            ArmorWriter armor_writer_;

            void WriteESK(OutStream &out);
        public:
            void Start(const SafeVector &passphrase, MessageConfig message_config, Salt salt);
            void Start(std::unique_ptr<SafeVector> passphrase, MessageConfig message_config, Salt salt);

            void Start(EncryptionKey encryption_key, MessageConfig message_config, Salt salt);
            void Start(std::unique_ptr<EncryptionKey> encryption_key, MessageConfig message_config, Salt salt);

            void Update(SafeVector& buf, bool finish);

            const EncryptionKey &GetEncryptionKey() const;
            const Salt &GetSalt() const;
            const MessageConfig &GetMessageConfig() const;

            MessageWriterImpl();
    };

    void MessageWriterImpl::Start(const SafeVector &passphrase, MessageConfig message_config, Salt salt)
    {
        if(salt.empty())
            salt = GenerateRandomSalt();

        std::unique_ptr<EncryptionKey> encryption_key = GenerateEncryptionKey(
                Passphrase(passphrase),
                message_config.GetCipherAlgo(),
                message_config.GetHashAlgo(),
                message_config.GetIterations(),
                salt);
        Start(std::move(encryption_key), message_config, salt);
    }

    void MessageWriterImpl::Start(std::unique_ptr<SafeVector> passphrase, MessageConfig message_config, Salt salt)
    {
        // Passphrase will be deleted after this method
        Start(*passphrase, message_config, salt);
    }

    void MessageWriterImpl::Start(EncryptionKey encryption_key, MessageConfig message_config, Salt salt)
    {
        std::unique_ptr<EncryptionKey> uptr(new EncryptionKey(encryption_key));
        Start(std::move(uptr), message_config, salt);
    }

    void MessageWriterImpl::Start(std::unique_ptr<EncryptionKey> encryption_key, MessageConfig message_config, Salt salt)
    {
        message_config_ = message_config;
        salt_ = salt;
        encryption_key_ = std::move(encryption_key);

        if(message_config_.GetArmor())
            armor_writer_.Start();

        auto it = packet_chain_.begin();
        *it = PacketType::Literal;
        if(message_config.GetCompression() != Compression::Uncompressed)
            *++it = PacketType::Compressed;
        *++it = PacketType::SymmetricIntegProtected;
    }

    const EncryptionKey &MessageWriterImpl::GetEncryptionKey() const
    {
        return *encryption_key_;

    }

    const Salt &MessageWriterImpl::GetSalt() const
    {
        return salt_;
    }

    const MessageConfig &MessageWriterImpl::GetMessageConfig() const
    {
        return message_config_;
    }

    void MessageWriterImpl::WriteESK(OutStream &out)
    {
        if(!write_esk_)
            return;
        WriteSymmetricKeyESK(message_config_, salt_, out);
        write_esk_ = false;
    }

    void MessageWriterImpl::Update(SafeVector& buf, bool finish)
    {
        assert(encryption_key_);

        SafeVector temp_buf;
        auto temp_out = MakeOutStream(temp_buf);

        SafeVector target_buf;
        auto target_stm = MakeOutStream(target_buf);

        if(message_config_.GetArmor())
        {
            WriteESK(*temp_out);
            armor_writer_.GetInStream().Push(temp_buf);
            armor_writer_.Write(*target_stm, false);
        }
        else
        {
            WriteESK(*target_stm);
        }

        temp_buf.swap(buf);
        auto it = packet_chain_.begin();
        for(;it != packet_chain_.end() && *it != PacketType::Unknown; it++)
        {
            auto *packet_writer = packet_writer_factory_.GetOrCreate(*it, message_config_, salt_, *encryption_key_);
            packet_writer->GetInStream().Push(temp_buf);
            temp_out->Reset();
            if(!finish)
                packet_writer->Write(*temp_out);
            else
                packet_writer->Finish(*temp_out);
        }

        if(message_config_.GetArmor())
        {
            armor_writer_.GetInStream().Push(temp_buf);
            temp_out->Reset();
            armor_writer_.Write(*temp_out, finish);
        }

        target_stm->Write(temp_buf.data(), temp_buf.size());
        buf.swap(target_buf);
    }

    MessageWriterImpl::MessageWriterImpl():
        write_esk_(true)
    {
        packet_chain_.fill(PacketType::Unknown);
    }

    // MessageWriter
    void MessageWriter::Start(const SafeVector &passphrase, MessageConfig message_config, Salt salt)
    {
        impl_->Start(passphrase, message_config, salt);
    }

    void MessageWriter::Start(std::unique_ptr<SafeVector> passphrase, MessageConfig message_config,
            Salt salt)
    {
        impl_->Start(std::move(passphrase), message_config, salt);
    }

    void MessageWriter::Start(EncryptionKey encryption_key, MessageConfig message_config, Salt salt)
    {
        impl_->Start(encryption_key, message_config, salt);
    }

    void MessageWriter::Start(std::unique_ptr<EncryptionKey> encryption_key, MessageConfig message_config, Salt salt)
    {
        impl_->Start(std::move(encryption_key), message_config, salt);
    }

    void MessageWriter::Update(SafeVector& buf)
    {
        impl_->Update(buf, false); //finish = false
    }

    void MessageWriter::Finish(SafeVector& buf)
    {
        impl_->Update(buf, true); //finish = true
    }

    const EncryptionKey &MessageWriter::GetEncryptionKey() const
    {
        return impl_->GetEncryptionKey();
    }

    const Salt &MessageWriter::GetSalt() const
    {
        return impl_->GetSalt();
    }

    const MessageConfig &MessageWriter::GetMessageConfig() const
    {
        return impl_->GetMessageConfig();
    }

    MessageWriter::MessageWriter()
        :impl_(new MessageWriterImpl()){}

    MessageWriter::~MessageWriter()
    {
        delete impl_;
    }

}
