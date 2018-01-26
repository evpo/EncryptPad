//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <map>
#include "message_config.h"
#include "memory_stream.h"
#include "packet_parsers.h"

namespace LibEncryptMsg
{
    void WriteSymmetricKeyESK(const MessageConfig &config, Salt salt, OutStream &out);

    class PacketWriter
    {
        public:
            PacketWriter(const MessageConfig &config, Salt salt, const EncryptionKey &key);
            void Write(OutStream &out);
            void Finish(OutStream &out);
            InBufferStream &GetInStream();
        protected:
            InBufferStream in_;
            InBufferStream out_;
            const MessageConfig &config_;
            const EncryptionKey &encryption_key_;
            Salt salt_;
            bool write_header_;
            bool finish_;

            void WriteHeader(OutStream &out);
            virtual void DoWriteHeader(OutStream &out) = 0;
            virtual void DoWrite(OutStream &out) = 0;
            virtual void DoFinish(OutStream &out) = 0;
    };

    class PacketWriterFactory
    {
        private:
            std::map<PacketType, std::unique_ptr<PacketWriter> > packet_map_;
        public:
            PacketWriter *GetOrCreate(PacketType packet_type, const MessageConfig &config, Salt salt,
                    const EncryptionKey &encryption_key);
    };
}

