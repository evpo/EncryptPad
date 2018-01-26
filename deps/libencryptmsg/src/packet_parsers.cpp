//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#include "packet_parsers.h"

namespace LibEncryptMsg
{
    size_t ReadLength(InBufferStream &stm, bool &is_partial_length)
    {
        is_partial_length = false;
        size_t ret_val = -1;

        unsigned char c = stm.Get();

        if(c < 192)
        {
            ret_val = c;
        }
        else if(c < 224)
        {
            ret_val = (c - 192) * 256;
            c = stm.Get();
            ret_val += c + 192;
        }
        else if(c == 255)
        {
            ret_val = static_cast<size_t>(stm.Get()) << 24;
            ret_val |= stm.Get() << 16;
            ret_val |= stm.Get() << 8;
            ret_val |= stm.Get();
        }
        else // Partial body length
        {
            is_partial_length = true;
            ret_val = 0;
            ret_val = 1 << (c & 0x1F);

            // partial length can be used with the following packets only
            // PKT_PLAINTEXT PKT_ENCRYPTED PKT_ENCRYPTED_MDC PKT_COMPRESSED
            // This is used for packets split in multiple packets. 
            // After this packet there will be another with its own length header.
        }
        return ret_val;
    }

    PacketHeader ReadPacketHeader(InBufferStream &stm)
    {
        PacketHeader ret_val = {};
        uint8_t c = stm.Get();
        if(c & 0x40) // new header
        {
            ret_val.is_new_format = true;
            ret_val.packet_type = static_cast<PacketType>(c & 0x3F);
            ret_val.body_length = ReadLength(stm, ret_val.is_partial_length);
        }
        else
        {
            ret_val.is_new_format = false;
            ret_val.packet_type = static_cast<PacketType>((c & 0x3F) >> 2);
            uint8_t len_bytes = ((c & 3) == 3) ? 0 : (1 << (c & 3));
            ret_val.body_length = ReadOldLength(stm, len_bytes);
            if(!ret_val.body_length)
                ret_val.is_partial_length = true;
        }
        return ret_val;
    }

    size_t ReadOldLength(InBufferStream &stm, uint8_t len_bytes)
    {
        if (!len_bytes)
        {
            // partial length can be used with the following packets only
            // PKT_PLAINTEXT PKT_ENCRYPTED PKT_ENCRYPTED_MDC PKT_COMPRESSED
            // maybe the calling code needs some indication that partial length was selected
            // This is used for packets split in multiple packets. 
            // After this packet there will be another with its own length header.

            return 0;
        }

        size_t ret_val = 0;
        for (; len_bytes; len_bytes--)
        {
            ret_val <<= 8;
            uint8_t b = stm.Get();
            ret_val |= b;
        }
        return ret_val;
    }

    const PacketSpec &GetPacketSpec(PacketType packet_type)
    {
        switch(packet_type)
        {
            case PacketType::SymmetricKeyESK:
                static PacketSpec p1 =
                {
                    packet_type,
                    "SymmetricKeyESK"
                };
                return p1;

            case PacketType::Symmetric:
                static PacketSpec p2 =
                {
                    packet_type,
                    "Symmetric"
                };
                return p2;
            case PacketType::SymmetricIntegProtected:
                static PacketSpec p3 =
                {
                    packet_type,
                    "SymmetricIntegProtected"
                };
                return p3;
            case PacketType::Compressed:
                static PacketSpec p4 =
                {
                    packet_type,
                    "Compressed"
                };
                return p4;
            case PacketType::MDC:
                static PacketSpec p5 =
                {
                    packet_type,
                    "MDC"
                };
                return p5;
            case PacketType::Literal:
                static PacketSpec p6 =
                {
                    packet_type,
                    "Literal"
                };
                return p6;

            case PacketType::Unknown:
            default:
                static PacketSpec p7 =
                {
                    PacketType::Unknown,
                    "Unknown"
                };
                return p7;
        };
    }
}
