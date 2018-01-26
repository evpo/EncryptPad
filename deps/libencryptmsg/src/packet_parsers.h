//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include "memory_stream.h"
#include <string>

namespace LibEncryptMsg
{
    enum class PacketType
    {
        Unknown = -1,
        SymmetricKeyESK = 3,
        Symmetric = 9,
        SymmetricIntegProtected = 18,
        Compressed = 8,
        MDC = 19,
        Literal = 11,
    };

    struct PacketHeader
    {
        PacketType packet_type;
        size_t body_length;
        bool is_new_format;
        bool is_partial_length;
        PacketHeader():
            packet_type(PacketType::Unknown),
            body_length(0),
            is_new_format(false),
            is_partial_length(false){}

    };
    const size_t kHeaderMaxSize = 6;
    PacketHeader ReadPacketHeader(InBufferStream &stm);
    size_t ReadLength(InBufferStream &stm, bool &is_partial_length);
    size_t ReadOldLength(InBufferStream &stm, uint8_t len_bytes);

    struct PacketSpec
    {
        PacketType packet_type;
        std::string packet_name;
    };

    const PacketSpec &GetPacketSpec(PacketType packet_type);
}
