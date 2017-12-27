//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//This file is part of EncryptPad
//
//EncryptPad is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 2 of the License, or
//(at your option) any later version.
//
//EncryptPad is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EncryptPad.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************
#include "wad_reader_writer.h"
#include <algorithm>
#include "assert.h"
#include "packet_stream.h"

// // throw exception on assert
// #ifdef EXCEPTION_ASSERT 
// #include <stdexcept> 
// #define STR_VALUE_OF_MACRO(arg) #arg
// #define STR_VALUE_OF_MACRO2(arg) STR_VALUE_OF_MACRO(arg)
// #define ASSERT(x) if (!(x)) {throw std::logic_error("Assertion failure in " __FILE__ " at line " STR_VALUE_OF_MACRO2(__LINE__) ": " #x "\n");} 
// #else 
// #include <cassert> 
// #define ASSERT(x) assert(x) 
// #endif

using namespace LibEncryptMsg;

namespace
{
    using namespace EncryptPad;

    const uint32_t kInvalid = -1; 

    bool ReadUint(InStream &in, uint32_t &val)
    {
        bool result = in.Read(reinterpret_cast<byte *>(&val), 4) == 4;
        //TODO: swap for big endian if needed
        return result;
    }

    void WriteUint(OutStream &out, uint32_t val)
    {
        //TODO: swap for big endian if needed
        out.Write(reinterpret_cast<byte *>(&val), 4);
    }

    bool WriteWadImpl(InStream &in, OutStream &out, const std::string &key_file)
    {
        // uint32_t dir_offset = 12 + payload_size + key_file.size(); // 12 - these 4 items

        // Header
        out.Write(reinterpret_cast<const byte*>("PWAD"), 4);
        uint32_t lumps = 2;
        WriteUint(out, lumps); // 2 lumps
        uint32_t dir_offset = 12;
        WriteUint(out, dir_offset);
        // Header end

        uint32_t header_size = out.GetCount();
        uint32_t directory_size = 2 * (4 + 4 + 8); // 2 records in the directory
        uint32_t key_file_offset = header_size + directory_size;
        uint32_t key_file_size = key_file.size();
        assert(dir_offset == out.GetCount());

        // Directory
        std::string lump_name = "__X2_KEY";
        WriteUint(out, key_file_offset);
        WriteUint(out, key_file_size);
        assert(lump_name.size() == 8);
        out.Write(reinterpret_cast<const byte*>(&lump_name[0]), 8);

        lump_name = "_PAYLOAD";
        uint32_t payload_size = in.GetCount();
        uint32_t payload_offset = header_size + directory_size + key_file_size;
        WriteUint(out, payload_offset);
        WriteUint(out, payload_size);
        assert(lump_name.size() == 8);
        out.Write(reinterpret_cast<const byte*>(&lump_name[0]), 8);
        // Directory end

        assert(key_file_offset == out.GetCount());

        // Key
        out.Write(reinterpret_cast<const byte*>(key_file.c_str()), key_file_size);

        assert(payload_offset == out.GetCount());

        // Payload
        int b;
        while((b = in.Get()) != -1)
        {
            out.Put(b);
        }

        return true;
    }

    struct WadMetadata
    {
        uint32_t dir_offset;
        uint32_t payload_offset;
        uint32_t payload_size;
        bool key_file_found;
        uint32_t key_file_offset;
        uint32_t key_file_size;
    };

    PacketResult ExtractWadMetadata(RandomInStream &stm, stream_length_type length, WadMetadata &metadata)
    {
        std::string wad_type(4, ' ');
        if(stm.Read(reinterpret_cast<byte *>(&wad_type[0]), 4) != 4)
            return PacketResult::InvalidWadFile;

        uint32_t num_lumps;
        if(!ReadUint(stm, num_lumps))
            return PacketResult::InvalidWadFile;

        if(!ReadUint(stm, metadata.dir_offset))
            return PacketResult::InvalidWadFile;

        if(wad_type != "PWAD" && wad_type != "IWAD")
            return PacketResult::InvalidWadFile;

        if(length - 1 < metadata.dir_offset)
            return PacketResult::InvalidWadFile;

        metadata.payload_offset = kInvalid;
        metadata.payload_size = 0;
        metadata.key_file_found = false;
        metadata.key_file_offset = kInvalid;
        metadata.key_file_size = 0;

        if(!stm.Seek(metadata.dir_offset))
        {
            return PacketResult::InvalidWadFile;
        }

        for(unsigned i = 0; i < num_lumps; i++)
        {
            uint32_t offset;
            uint32_t size;
            std::string name(8, ' ');

            if(!ReadUint(stm, offset))
                return PacketResult::InvalidWadFile;

            if(!ReadUint(stm, size))
                return PacketResult::InvalidWadFile;

            if(stm.Read(reinterpret_cast<byte *>(&name[0]), 8) != 8)
                return PacketResult::InvalidWadFile;

            std::transform(std::begin(name), std::end(name), std::begin(name), ::toupper);

            if(name == "__X2_KEY")
            {
                metadata.key_file_found = true;
                metadata.key_file_offset = offset;
                metadata.key_file_size = size;
            }
            else
            {
                metadata.payload_offset = offset;
                metadata.payload_size = size;
            }
        }

        if(metadata.payload_offset == kInvalid || !metadata.key_file_found)
            return PacketResult::InvalidWadFile;

        return PacketResult::Success;
    }
}

namespace EncryptPad
{
    bool WriteWad(InStream &in, OutStream &out, const std::string &key_file)
    {
        return WriteWadImpl(in, out, key_file);
    }

    PacketResult ExtractKeyFromWad(RandomInStream &in, std::string &key_file)
    {
        WadMetadata metadata;
        key_file.clear();
        auto result = ExtractWadMetadata(in, in.GetCount(), metadata);
        if(result != PacketResult::Success)
            return result;

        if(!metadata.key_file_found)
            return PacketResult::InvalidWadFile;

        if(metadata.key_file_size <= 0)
            return PacketResult::Success;

        if(!in.Seek(metadata.key_file_offset))
            return PacketResult::InvalidWadFile;

        key_file.resize(metadata.key_file_size);
        if(in.Read(reinterpret_cast<byte *>(&key_file[0]), metadata.key_file_size) != metadata.key_file_size)
            return PacketResult::InvalidWadFile;

        return PacketResult::Success;
    }

    PacketResult ExtractFromWad(RandomInStream &in, OutStream &out, std::string &key_file)
    {
        WadMetadata metadata;
        auto result = ExtractWadMetadata(in, in.GetCount(), metadata);
        in.Seek(metadata.payload_offset);

        if(result != PacketResult::Success)
            return result;

        Botan::SecureVector<byte> buffer;
        buffer.resize(metadata.payload_size);
        in.Read(buffer.data(), metadata.payload_size);
        out.Write(buffer.data(), buffer.size());

        key_file.clear();

        if(metadata.key_file_offset != kInvalid)
        {
            key_file.resize(metadata.key_file_size);
            in.Seek(metadata.key_file_offset);
            in.Read(reinterpret_cast<byte *>(&*key_file.begin()), metadata.key_file_size);
        }

        return PacketResult::Success;
    }
}
