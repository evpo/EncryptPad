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

using namespace EncryptMsg;

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

    bool WriteWadHeadImpl(const std::string &key_file, uint32_t payload_size, const std::string &version, OutStream &out)
    {
        // Version 0 does not support undefined payload size
        assert(payload_size > 0 || version != "0");

        // Header
        out.Write(reinterpret_cast<const byte*>("PWAD"), 4);
        uint32_t lumps = 3;
        WriteUint(out, lumps);
        uint32_t dir_offset = 12;
        WriteUint(out, dir_offset);
        // Header end

        uint32_t header_size = out.GetCount();
        uint32_t directory_size = lumps * (4 + 4 + 8);
        assert(dir_offset == out.GetCount());

        // Directory
        std::string lump_name = "__X2_KEY";
        uint32_t offset = header_size + directory_size;
        uint32_t size = key_file.size();
        WriteUint(out, offset);
        WriteUint(out, size);
        assert(lump_name.size() == 8);
        out.Write(reinterpret_cast<const byte*>(&lump_name[0]), 8);
        offset += size;

        lump_name = "_VERSION";
        size = version.size();
        WriteUint(out, offset);
        WriteUint(out, size);
        assert(lump_name.size() == 8);
        out.Write(reinterpret_cast<const byte*>(&lump_name[0]), 8);
        offset += size;

        lump_name = "_PAYLOAD";
        size = payload_size;
        WriteUint(out, offset);
        WriteUint(out, size);
        assert(lump_name.size() == 8);
        out.Write(reinterpret_cast<const byte*>(&lump_name[0]), 8);
        offset += size;

        // Directory end

        // Key
        out.Write(reinterpret_cast<const byte*>(key_file.c_str()), key_file.size());

        // Version
        out.Write(reinterpret_cast<const byte*>(version.c_str()), version.size());

        return true;
    }

    bool WriteWadImpl(InStream &in, OutStream &out, const std::string &key_file)
    {
        std::string version = "0";
        if(!WriteWadHeadImpl(key_file, in.GetCount(), version, out))
            return false;

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
        bool version_found;
        uint32_t version_offset;
        uint32_t version_size;
    };

    EpadResult ExtractWadMetadata(RandomInStream &stm, stream_length_type length, WadMetadata &metadata)
    {
        std::string wad_type(4, ' ');
        if(stm.Read(reinterpret_cast<byte *>(&wad_type[0]), 4) != 4)
            return EpadResult::InvalidOrIncompleteWadFile;

        uint32_t num_lumps;
        if(!ReadUint(stm, num_lumps))
            return EpadResult::InvalidOrIncompleteWadFile;

        if(!ReadUint(stm, metadata.dir_offset))
            return EpadResult::InvalidOrIncompleteWadFile;

        if(wad_type != "PWAD" && wad_type != "IWAD")
            return EpadResult::InvalidWadFile;

        if(length - 1 < metadata.dir_offset)
            return EpadResult::InvalidOrIncompleteWadFile;

        metadata.payload_offset = kInvalid;
        metadata.payload_size = 0;
        metadata.key_file_found = false;
        metadata.key_file_offset = kInvalid;
        metadata.key_file_size = 0;

        if(!stm.Seek(metadata.dir_offset))
        {
            return EpadResult::InvalidOrIncompleteWadFile;
        }

        for(unsigned i = 0; i < num_lumps; i++)
        {
            uint32_t offset;
            uint32_t size;
            std::string name(8, ' ');

            if(!ReadUint(stm, offset))
                return EpadResult::InvalidOrIncompleteWadFile;

            if(!ReadUint(stm, size))
                return EpadResult::InvalidOrIncompleteWadFile;

            if(stm.Read(reinterpret_cast<byte *>(&name[0]), 8) != 8)
                return EpadResult::InvalidOrIncompleteWadFile;

            std::transform(std::begin(name), std::end(name), std::begin(name), ::toupper);

            if(name == "__X2_KEY")
            {
                metadata.key_file_found = true;
                metadata.key_file_offset = offset;
                metadata.key_file_size = size;
            }
            else if(name == "_VERSION")
            {
                metadata.version_found = true;
                metadata.version_offset = offset;
                metadata.version_size = size;
            }
            else
            {
                metadata.payload_offset = offset;
                metadata.payload_size = size;
            }
        }

        if(metadata.payload_offset == kInvalid || !metadata.key_file_found)
            return EpadResult::InvalidWadFile;

        return EpadResult::Success;
    }
}

namespace EncryptPad
{
    //TODO: The function is not used
    bool WriteWad(InStream &in, OutStream &out, const std::string &key_file)
    {
        return WriteWadImpl(in, out, key_file);
    }

    bool WriteWadHead(const std::string &key_file, uint32_t payload_size, const std::string &version, OutStream &out)
    {
        return WriteWadHeadImpl(key_file, payload_size, version, out);
    }

    EpadResult ParseWad(RandomInStream &in, std::string &key_file, uint32_t &payload_offset, uint32_t &payload_size)
    {
        WadMetadata metadata;
        key_file.clear();
        auto result = ExtractWadMetadata(in, in.GetCount(), metadata);
        if(result != EpadResult::Success)
            return result;

        payload_offset = metadata.payload_offset;
        payload_size = metadata.payload_size;

        if(!metadata.key_file_found)
            return EpadResult::InvalidWadFile;

        if(metadata.key_file_size <= 0)
            return EpadResult::Success;

        if(!in.Seek(metadata.key_file_offset))
            return EpadResult::InvalidOrIncompleteWadFile;

        key_file.resize(metadata.key_file_size);
        if(in.Read(reinterpret_cast<byte *>(&key_file[0]), metadata.key_file_size) != metadata.key_file_size)
            return EpadResult::InvalidOrIncompleteWadFile;

        if(!in.Seek(metadata.payload_offset))
            return EpadResult::InvalidOrIncompleteWadFile;

        return EpadResult::Success;
    }
    EpadResult ExtractKeyFromWad(RandomInStream &in, std::string &key_file)
    {
        WadMetadata metadata;
        key_file.clear();
        auto result = ExtractWadMetadata(in, in.GetCount(), metadata);
        if(result != EpadResult::Success)
            return result;

        if(!metadata.key_file_found)
            return EpadResult::InvalidWadFile;

        if(metadata.key_file_size <= 0)
            return EpadResult::Success;

        if(!in.Seek(metadata.key_file_offset))
            return EpadResult::InvalidWadFile;

        key_file.resize(metadata.key_file_size);
        if(in.Read(reinterpret_cast<byte *>(&key_file[0]), metadata.key_file_size) != metadata.key_file_size)
            return EpadResult::InvalidWadFile;

        return EpadResult::Success;
    }

    EpadResult ExtractFromWad(RandomInStream &in, OutStream &out, std::string &key_file)
    {
        WadMetadata metadata;
        auto result = ExtractWadMetadata(in, in.GetCount(), metadata);
        in.Seek(metadata.payload_offset);

        if(result != EpadResult::Success)
            return result;

        Botan::SecureVector<byte> buffer;
        buffer.resize(metadata.payload_size != 0 ? metadata.payload_size : in.GetCount());
        in.Read(buffer.data(), buffer.size());
        out.Write(buffer.data(), buffer.size());

        key_file.clear();

        if(metadata.key_file_offset != kInvalid)
        {
            key_file.resize(metadata.key_file_size);
            in.Seek(metadata.key_file_offset);
            in.Read(reinterpret_cast<byte *>(&*key_file.begin()), metadata.key_file_size);
        }

        return EpadResult::Success;
    }
}
