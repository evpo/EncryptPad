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
#include "packet_composer.h"
#include <array>
#include "encryption_pipeline.h"
#include "zlib.h"
#include "epad_utilities.h"
#include "openpgp_conversions.h"
#include "key_generation.h"

namespace EncryptPad
{
    typedef Botan::SecureVector<byte> Buffer;
    typedef std::unique_ptr<InPacketStreamMemory> BufferPtr;
    const int kCompressionBufferSize = 4096;
    const int kCopyBufferSize = 4096;
    
    // Packet Reader Writer

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
        stream_length_type body_length;
        bool is_new_format;
        bool is_partial_length;
        PacketHeader():
            packet_type(PacketType::Unknown),
            body_length(0),
            is_new_format(false),
            is_partial_length(false){}

    };

    class PacketRWBase
    {
    public:
        PacketRWBase(PacketMetadata &metadata):metadata_(metadata){}
        // Reads packet body and writes its payload while setting metadata_out fields
        // Payload is the contained packets or empty if there are no packets inside
        PacketResult Read()
        {
            return DoRead();
        }

    protected:
        PacketMetadata &metadata_;
        virtual PacketResult DoRead() = 0;
    };

    // PacketRWBase implementaions
    class SymmetricRWBase : public PacketRWBase
    {
    public:
        SymmetricRWBase(InStream &in, OutStream &out, 
            PacketMetadata &metadata, const EncryptParams &enc_params):PacketRWBase(metadata), 
            in_(in), out_(out), encrypt_params_(enc_params), cipher_(nullptr){}

    protected:
        InStream &in_;
        OutStream &out_;
        const EncryptParams &encrypt_params_;
        Botan::Pipe pipe_;
        Botan::Keyed_Filter *cipher_;
        Botan::SecureVector<byte> prefix_;

        PacketResult ReadAndCheckPrefix(Botan::Pipe &pipe, int prefix_len);
        PacketResult InitRead();
    };

    class SymmetricIntegProtectedRW : public SymmetricRWBase
    {
    public:
        SymmetricIntegProtectedRW(InStream &in, OutStream &out, 
            PacketMetadata &metadata, const EncryptParams &enc_params) 
            : SymmetricRWBase(in, out, metadata, enc_params){} 

    protected:
        PacketResult DoRead() override;
    };

    class SymmetricRW : public SymmetricRWBase
    {
    public:
        SymmetricRW(InStream &in, OutStream &out, 
            PacketMetadata &metadata, const EncryptParams &enc_params) 
            : SymmetricRWBase(in, out, metadata, enc_params){} 

    protected:
        PacketResult DoRead() override;
    };

    class SymmetricKeyESKRW : public PacketRWBase
    {
    public:
        SymmetricKeyESKRW(InStream &in, PacketMetadata &metadata)
            :PacketRWBase(metadata), in_(in){}

    private:
        InStream &in_;

    protected:
        PacketResult DoRead() override;
    };

    class CompressedRW : public PacketRWBase
    {
    public:
        CompressedRW(InStream &in, OutStream &out, PacketMetadata &metadata)
            :PacketRWBase(metadata), in_(in), out_(out) {}

    private:
        InStream &in_;
        OutStream &out_;

    protected:
        PacketResult DoRead() override;
    };

    class LiteralRW : public PacketRWBase
    {
    public:
        LiteralRW(InStream &in, OutStream &out, PacketMetadata &metadata)
            :PacketRWBase(metadata), in_(in), out_(out) {}

    private:
        InStream &in_;
        OutStream &out_;

    protected:
        PacketResult DoRead() override;
    };

    // end PacketRWbase implementations

    // Implementation

    stream_length_type ReadLength(InStream &stm, bool &is_partial_length)
    {
        is_partial_length = false;
        stream_length_type ret_val = -1;

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
            ret_val = static_cast<stream_length_type>(stm.Get()) << 24;
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

    struct PartialLengthEOFHandler : public EOFHandlerBase
    {
    public:
        PartialLengthEOFHandler()
            :full_length(0), body_length(0), is_partial(true)
        {}

        stream_length_type full_length;
        stream_length_type body_length;
        bool is_partial;

        virtual void operator()(InStream &stm)
        {
            full_length -= body_length;
            assert(full_length >= 0);
            if(full_length == 0 || !is_partial)
                return;
            stm.SetCount(full_length);
            body_length = ReadLength(stm, is_partial);
            full_length = stm.GetCount();
            assert(full_length >= 0);
            stm.SetCount(body_length);
        }
    };

    void WriteLength(OutStream &stm, stream_length_type len)
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

    void WriteHeader(OutStream &out, const PacketHeader &header)
    {
        // Max length 0xffffffff - 0xffff - 1, which is about 4 gb
        assert(header.body_length < (0xffffffff - 0xffff - 1));
        byte c = 0x80;
        if(header.is_new_format)
            c |= 0x40;

        byte t = static_cast<byte>(header.packet_type);
        c |= header.is_new_format ? t : t << 2;
        if(!header.is_new_format && header.is_partial_length)
        {
            c |= 0x03;
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

    void WriteLiteralPrefix(OutStream &out, const PacketMetadata &metadata)
    {
        out.Put(metadata.is_binary ? 'b' : 't');
        out.Put(static_cast<byte>(metadata.file_name.size()));
        out.Write(reinterpret_cast<const byte*>(metadata.file_name.data()), metadata.file_name.size());
        out.Write(reinterpret_cast<const byte*>(&metadata.file_date), 4);
    }

    void WriteLiteral(InStream &in, OutStream &out, const PacketMetadata &metadata)
    {
        WriteLiteralPrefix(out, metadata);
        Buffer buffer;
        buffer.resize(kCopyBufferSize);
        
        while(!in.IsEOF())
        {
            stream_length_type bytes = in.Read(buffer.begin(), kCopyBufferSize);
            out.Write(buffer.begin(), bytes);
        }
    }

    PacketResult WriteCompressed(InStream &in, OutStream &out, const PacketMetadata &metadata)
    {
        out.Put(static_cast<byte>(metadata.compression));
        //TODO: the code below is 70% similar to the inflating code. I will need to merge it somehow
        z_stream zs;
        memset(&zs, 0, sizeof(zs));
        const int level = Z_DEFAULT_COMPRESSION;
        const int mem_level = 8;
        int result = Z_OK;
        switch(metadata.compression)
        {
            case Compression::ZIP:
                result = deflateInit2(&zs, level, Z_DEFLATED, -13, mem_level, Z_DEFAULT_STRATEGY);
                break;
            case Compression::ZLIB:
                result = deflateInit(&zs, level);
                break;
            default:
                //unsupported
                return PacketResult::UnsupportedCompressionAlgo;
        }

        if(result != Z_OK)
            return PacketResult::CompressionError;

        std::array<byte, kCompressionBufferSize> buffer_in;
        std::array<byte, kCompressionBufferSize> buffer_out;

#ifndef NDEBUG 
        stream_length_type debug_test_counter = out.GetCount();
#endif
        int flush = Z_NO_FLUSH;

        while(!in.IsEOF())
        {
            int bytes_read = in.Read(buffer_in.begin(), buffer_in.size());
            if(in.IsEOF())
                flush = Z_FINISH;

            zs.next_in = buffer_in.begin();
            zs.avail_in = bytes_read;
            do
            {
                zs.next_out = buffer_out.begin();
                zs.avail_out = buffer_out.size();
                result = deflate(&zs, flush);
                switch(result)
                {
                    case Z_OK:
                    case Z_STREAM_END:
                    case Z_BUF_ERROR:
                        break;
                    default:
                        return PacketResult::CompressionError;
                }
                out.Write(buffer_out.begin(), buffer_out.size() - zs.avail_out);
            }
            while(result == Z_OK && !zs.avail_out);
        }

        assert(result == Z_STREAM_END);
        assert(zs.avail_in == 0);
        result = deflateEnd(&zs);

        if(result != Z_OK)
            return PacketResult::CompressionError;

        assert(out.GetCount() - debug_test_counter == zs.total_out); 
        return PacketResult::Success;
    }

    void WriteSymmetricKeyESK(OutStream &out, const KeyRecord &key_record, const PacketMetadata &metadata)
    {
        out.Put(4);
        out.Put(static_cast<byte>(metadata.cipher_algo));
        out.Put(3); //salt and iterations 
        out.Put(static_cast<byte>(metadata.hash_algo));
        assert(key_record.salt.size() == 8);
        out.Write(key_record.salt.begin(), key_record.salt.size());
        out.Put(EncodeS2KIterations(key_record.iterations));
    }

    void WriteEncryptedIntegProtected(InStream &in, OutStream &out, 
            const KeyRecord &key_record, const PacketMetadata &metadata)
    {
        using namespace Botan;

        //Prepare encryption pipe
        SecureVector<byte> iv;
        auto algo_spec = GetAlgoSpec(metadata.cipher_algo);
        iv.resize(algo_spec.block_size);
        std::fill(iv.begin(), iv.end(), 0);
        auto cipher = Botan::get_cipher(algo_spec.botan_name, key_record.key, OctetString(iv), ENCRYPTION);
        Pipe pipe;
        pipe.append(cipher);
        pipe.start_msg();

        Buffer buffer;
        auto buffer_out = MakeOutStream(buffer);

        SHA_160 hash;
        SecureVector<byte> prefix;
        prefix.resize(algo_spec.block_size);
        GenerateNewKey(prefix.begin(), prefix.size());
        prefix.push_back(*(prefix.begin() + prefix.size() - 2));
        prefix.push_back(*(prefix.begin() + prefix.size() - 2));
        hash.update(prefix.begin(), prefix.size());

        pipe.write(prefix.begin(), prefix.size());

        Buffer copy_buf;
        copy_buf.resize(kCopyBufferSize);
        while(!in.IsEOF())
        {
            stream_length_type copied = in.Read(copy_buf.begin(), kCopyBufferSize);
            hash.update(copy_buf.begin(), copied);
            pipe.write(copy_buf.begin(), copied);
        }

        hash.update(0xD3);
        hash.update(0x14);
        pipe.write(0xD3);
        pipe.write(0x14);
        SecureVector<byte> sha1 = hash.final();
        assert(sha1.size() == 20);
        pipe.write(sha1.begin(), sha1.size());
        pipe.end_msg();

        out.Put(1); // packet version
        while(!pipe.end_of_data())
        {
            stream_length_type bytes_read = pipe.read(copy_buf.begin(), kCopyBufferSize);
            out.Write(copy_buf.begin(), bytes_read);
        }
    }

    // Main Packet Methods
    PacketResult WriteAllPackets(InStream &in, OutStream &out, PacketMetadata &metadata, EncryptParams &encrypt_params)
    {
        Buffer buffer;
        Buffer tmp_buf;

        auto buffer_out = MakeOutStream(buffer);
        BufferPtr buffer_in(nullptr);
        BufferPtr tmp_in(nullptr);
        auto tmp_out = MakeOutStream(tmp_buf);

        // *** SymmetricKeyESK ***
        const KeyRecord &key_record = encrypt_params.key_service->GetKeyForSaving();
        if(key_record.IsEmpty())
            return PacketResult::KeyIsRequiredForSaving;

          //ESK -> buffer
        WriteSymmetricKeyESK(*buffer_out, key_record, metadata);
        PacketHeader header = {};
        header.body_length = buffer_out->GetCount();
        header.packet_type = PacketType::SymmetricKeyESK;
        header.is_new_format = true;
          //ESK header -> out
        WriteHeader(out, header);
          //buffer with ESK -> out
        out.Write(buffer.begin(), buffer_out->GetCount());

        // *** Literal ***
          //literal prefix -> tmp_buf
        WriteLiteralPrefix(*tmp_out, metadata);
        header = {};
        header.packet_type = PacketType::Literal;
        header.body_length = tmp_out->GetCount() + in.GetCount();
        header.is_new_format = false;
        header.is_partial_length = true;

        buffer_out->Reset();
          //literal header -> buffer
        WriteHeader(*buffer_out, header);
          //literal prefix -> buffer
        buffer_out->Write(tmp_buf.begin(), tmp_out->GetCount());

          //clear tmp_buf
        tmp_out->Reset();

        InPacketStreamPipe pipe_in;
        
        // *** Compressed ***
        if(metadata.compression != Compression::Uncompressed)
        {
            buffer_in = BufferPtr(new InPacketStreamMemory(buffer.begin(), buffer.begin() + buffer_out->GetCount()));
            pipe_in.Push(*buffer_in);
            pipe_in.Push(in);
              //Compressed -> tmp_buf
            auto result = WriteCompressed(pipe_in, *tmp_out, metadata);
            if(result != PacketResult::Success)
                return result;

              //Clear buffer
            buffer_out->Reset();
            header = {};
            header.packet_type = PacketType::Compressed;
            header.body_length = tmp_out->GetCount();
            header.is_new_format = false;
            header.is_partial_length = true;

              //Compressed header -> buffer
            WriteHeader(*buffer_out, header);
            buffer_in = BufferPtr(new InPacketStreamMemory(buffer.begin(), buffer.begin() + buffer_out->GetCount()));
            tmp_in = BufferPtr(new InPacketStreamMemory(tmp_buf.begin(), tmp_buf.begin() + tmp_out->GetCount()));
              //Compressed header and Compressed pushed to pipe
            pipe_in.Clear();
            pipe_in.Push(*buffer_in);
            pipe_in.Push(*tmp_in);
        }
        else
        {
            buffer_in = BufferPtr(new InPacketStreamMemory(buffer.begin(), buffer.begin() + buffer_out->GetCount()));
              //Literal header, prefix and raw data pushed to pipe
            pipe_in.Push(*buffer_in);
            pipe_in.Push(in);
        }

        // *** EncryptedIntegProtected ***
        Buffer integ_buf;
        auto integ_buf_out = MakeOutStream(integ_buf);
          //IntegProtected -> integ_buf
        WriteEncryptedIntegProtected(pipe_in, *integ_buf_out, key_record, metadata);
        assert(integ_buf_out->GetCount() > 0);
        assert(pipe_in.GetCount() == 0);

        header = {};
        header.packet_type = PacketType::SymmetricIntegProtected;
        header.body_length = integ_buf_out->GetCount();
        //TODO: implement new format partial length
        header.is_new_format = true;
        header.is_partial_length = false;
          // header -> out
        WriteHeader(out, header);
        out.Write(integ_buf.begin(), integ_buf_out->GetCount());

        return PacketResult::Success;
    }

    stream_length_type ReadOldLength(InStream &stm, byte len_bytes)
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

        stream_length_type ret_val = 0;
        for (; len_bytes; len_bytes--)
        {
            ret_val <<= 8;
            byte b = stm.Get();
            ret_val |= b;
        }
        return ret_val;
    }

    PacketHeader ReadPacketHeader(InStream &stm)
    {
        PacketHeader ret_val = {};
        byte c = stm.Get();
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
            byte len_bytes = ((c & 3) == 3) ? 0 : (1 << (c & 3));
            ret_val.body_length = ReadOldLength(stm, len_bytes);
            if(!ret_val.body_length)
                ret_val.is_partial_length = true;
        }
        return ret_val;
    }

    std::unique_ptr<PacketRWBase> PacketRWFactory(PacketType packet_type, InStream &in, OutStream &out, 
            PacketMetadata &metadata, const EncryptParams &encrypt_params)
    {
        switch(packet_type)
        {
            case PacketType::SymmetricKeyESK:
                return std::unique_ptr<PacketRWBase>(new SymmetricKeyESKRW(in, metadata));
            case PacketType::SymmetricIntegProtected:
                return std::unique_ptr<PacketRWBase>(new SymmetricIntegProtectedRW(in, out, metadata, encrypt_params));
            case PacketType::Symmetric:
                return std::unique_ptr<PacketRWBase>(new SymmetricRW(in, out, metadata, encrypt_params));
            case PacketType::Compressed:
                return std::unique_ptr<PacketRWBase>(new CompressedRW(in, out, metadata));
            case PacketType::Literal:
                return std::unique_ptr<PacketRWBase>(new LiteralRW(in, out, metadata));
            default:
                // unknown packet type
                return std::unique_ptr<PacketRWBase>();
        }
    }

    PacketResult ReadAllPackets(InStream &in, OutStream &out, PacketMetadata &metadata, const EncryptParams &encrypt_params)
    {
        if(in.GetCount() == 0)
            return PacketResult::Empty;

        Buffer high_buffer;
        Buffer low_buffer;
        bool low2high = true;
        auto high_out = MakeOutStream(high_buffer);
        auto low_out = MakeOutStream(low_buffer);
        InPacketStreamMemory high_in;
        InPacketStreamMemory low_in;
        InStream *in_ptr = &in;
        OutStream *out_ptr = high_out.get();

        while(in_ptr && !in_ptr->IsEOF())
        {
            PacketHeader header = ReadPacketHeader(*in_ptr);

            if(header.packet_type == PacketType::Unknown)
                return PacketResult::UnsupportedPacketType;

            stream_length_type previous_count = in_ptr->GetCount();
            stream_length_type body_length = header.is_partial_length && !header.is_new_format
                ? previous_count : header.body_length;

            in_ptr->SetCount(body_length);
            PacketResult result = PacketResult::UnexpectedError;

            std::unique_ptr<PacketRWBase> packet_rw;
            if(header.packet_type == PacketType::Literal)
            {
                packet_rw = PacketRWFactory(header.packet_type, *in_ptr, out, metadata, encrypt_params);
            }
            else
            {
                packet_rw = PacketRWFactory(header.packet_type, *in_ptr, *out_ptr, metadata, encrypt_params);
            }

            if(!packet_rw.get())
                return PacketResult::UnsupportedPacketType;

            if(header.is_partial_length && header.is_new_format)
            {
                PartialLengthEOFHandler eof_handler;
                eof_handler.full_length = previous_count;
                eof_handler.body_length = body_length;
                in_ptr->SetCount(body_length);

                auto &&clean_up_lambda = [&in_ptr](){in_ptr->SetEOFHandler(nullptr);};
                CleanUp<decltype(clean_up_lambda)> clean_up(clean_up_lambda);
                in_ptr->SetEOFHandler(&eof_handler);

                result = packet_rw->Read();
                in_ptr->SetCount(eof_handler.full_length);
            }
            else
            {
                result = packet_rw->Read();
                in_ptr->SetCount(previous_count - body_length);
            }

            if(result == PacketResult::Empty)
                continue;

            if(result != PacketResult::Success)
                return result;

            if(header.packet_type == PacketType::Literal)
                continue;

            if(in_ptr->GetCount() > 0)
                continue;

            // Change buffer direction
            if(low2high)
            {
                high_in.Set(high_buffer.begin(), high_buffer.begin() + out_ptr->GetCount());
                in_ptr = &high_in;
                low_out->Reset();
                out_ptr = low_out.get();
            }
            else
            {
                // high to low
                low_in.Set(low_buffer.begin(), low_buffer.begin() + out_ptr->GetCount());
                in_ptr = &low_in;
                high_out->Reset();
                out_ptr = high_out.get();
            }

            low2high = !low2high;
        }

        return PacketResult::Success;
    }


    // End of Main Packet Methods

    PacketResult ReadPacketImpl(InStream &in, OutStream &out, 
            const EncryptParams &enc_params, PacketMetadata &metadata)
    {
        // Compression will not be set if the file is not compressed. We set it to uncompressed now.
        metadata.compression = Compression::Uncompressed;
        return ReadAllPackets(in, out, metadata, enc_params);
    }

    // Public interface

    PacketResult ReadPacket(InStream &in, OutStream &out, 
            const EncryptParams &enc_params, PacketMetadata &metadata)
    {
        return ReadPacketImpl(in, out, enc_params, metadata);
    }

    PacketResult WritePacket(InStream &in, OutStream &out, 
            EncryptParams &enc_params, PacketMetadata &metadata)
    {
        return WriteAllPackets(in, out, metadata, enc_params);
    }
    // End Public interface

    // Packet methods implementations
    PacketResult SymmetricKeyESKRW::DoRead()
    {
        if(in_.Get()!=4)
            return PacketResult::UnexpectedFormat;

        metadata_.cipher_algo = static_cast<CipherAlgo>(in_.Get());
        switch(metadata_.cipher_algo)
        {
            case CipherAlgo::AES128:
            case CipherAlgo::TripleDES:
            case CipherAlgo::CAST5:
            case CipherAlgo::AES256:
                break;
            default:
                return PacketResult::UnsupportedAlgo;
        }

        if(in_.Get() != 3)
            return PacketResult::UnsupportedS2K;

        metadata_.hash_algo = static_cast<HashAlgo>(in_.Get());
        switch(metadata_.hash_algo)
        {
            case HashAlgo::SHA160:
            case HashAlgo::SHA256:
                break;
            default:
                return PacketResult::UnsupportedS2K;
        }

        metadata_.salt.resize(8);
        if(in_.Read(metadata_.salt.begin(), 8) != 8)
            return PacketResult::UnexpectedFormat;

        metadata_.iterations = DecodeS2KIterations(in_.Get());
        // return empty because this packet doesn't have children
        return PacketResult::Empty;
    }

    PacketResult SymmetricRW::DoRead()
    {
        using namespace Botan;
        auto result = InitRead();
        if(result != PacketResult::Success)
            return result;

        Pipe &pipe = pipe_;

        auto algo_spec = GetAlgoSpec(metadata_.cipher_algo);

        // Read the random iv to the same iv buffer. Add two check bytes as described in RFC4880
        int prefix_len = algo_spec.block_size + 2;
        if(in_.GetCount() < prefix_len)
        {
            return PacketResult::UnexpectedFormat;
        }

        SecureVector<byte> enc_prefix; //encrypted prefix
        enc_prefix.resize(prefix_len);
        in_.Read(enc_prefix.begin(), prefix_len);

        pipe.start_msg();
        WriteToPipe(enc_prefix, pipe);
        pipe.end_msg();

        auto prefix_result = ReadAndCheckPrefix(pipe, prefix_len);
        if( prefix_result != PacketResult::Success)
            return prefix_result;

        // resync the cipher
        SecureVector<byte> iv;
        iv.resize(algo_spec.block_size);
        std::copy(enc_prefix.begin() + 2, enc_prefix.end(), iv.begin());

        cipher_->set_iv(OctetString(iv));
        pipe.start_msg();
        WriteToPipe(in_, pipe);
        pipe.end_msg();

        assert(pipe.message_count() == 2);
        pipe.set_default_msg(1);

        Buffer buffer;
        buffer.resize(kCopyBufferSize);
        while (!pipe.end_of_data())
        {
            auto read_bytes = pipe.read(buffer.begin(), kCopyBufferSize);
            out_.Write(buffer.begin(), read_bytes);
        }

        return PacketResult::Success;
    }

    PacketResult SymmetricRWBase::ReadAndCheckPrefix(Botan::Pipe &pipe, int prefix_len)
    {
        using namespace Botan;
        prefix_.resize(prefix_len);
        int count = pipe.read(prefix_.begin(), prefix_len);
        if(count != prefix_len)
            return PacketResult::InvalidSurrogateIV;

        // check the last two bytes for integrity of the message
        byte *ptr = prefix_.begin();
        ptr += prefix_len - 4;
        if(*ptr != *(ptr + 2)) 
            return PacketResult::InvalidSurrogateIV;
        if(*(ptr + 1) != *(ptr + 3))
            return PacketResult::InvalidSurrogateIV;

        return PacketResult::Success;
    }

    PacketResult SymmetricRWBase::InitRead()
    {
        using namespace Botan;
        auto algo_spec = GetAlgoSpec(this->metadata_.cipher_algo);
        const std::string *passphrase = this->encrypt_params_.passphrase;
        const KeyRecord *key_record = nullptr;
        
        if(passphrase)
        {
            key_record = &this->encrypt_params_.key_service->ChangePassphrase(
                    *passphrase,
                    this->metadata_.hash_algo,
                    algo_spec.key_size,
                    this->metadata_.salt,
                    this->metadata_.iterations);
        }
        else
        {
            key_record = &this->encrypt_params_.key_service->GetKeyForLoading(
                    this->metadata_.salt, this->metadata_.iterations, this->metadata_.hash_algo);
        }

        if(key_record->IsEmpty())
            return PacketResult::InvalidPassphrase;

        SecureVector<byte> iv;
        iv.resize(algo_spec.block_size);
        std::fill(iv.begin(), iv.end(), 0);

        cipher_ = Botan::get_cipher(algo_spec.botan_name, key_record->key, OctetString(iv), DECRYPTION);
        pipe_.append(cipher_);

        return PacketResult::Success;
    }

    PacketResult SymmetricIntegProtectedRW::DoRead()
    {
        using namespace Botan;
        auto result = this->InitRead();
        if(result != PacketResult::Success)
            return result;

        Pipe &pipe = this->pipe_;
        byte b = 0;
        b = this->in_.Get();

        if(b != 1)
            return PacketResult::UnexpectedFormat;

        auto algo_spec = GetAlgoSpec(this->metadata_.cipher_algo);

        // Read the random iv to the same iv buffer. Add two check bytes as described in RFC4880
        int prefix_len = algo_spec.block_size + 2;

        if(this->in_.GetCount() < prefix_len)
            return PacketResult::UnexpectedFormat;

        pipe.start_msg();
        WriteToPipe(this->in_, pipe);
        pipe.end_msg();

        auto prefix_result = this->ReadAndCheckPrefix(pipe, prefix_len);
        if( prefix_result != PacketResult::Success)
            return prefix_result;

        // RFC 4880 (page 50)
        // After decryption the data is prefixed with an octet string, which length equals to the cipher block size;
        // and also two bytes that repeat the last two bytes of the octet string.
        // The prefix, the payload data and two bytes 0xD3 0x14 (encoding and length of MDC packet) are 
        // hashed
        SHA_160 hash;
        hash.update(this->prefix_.begin(), this->prefix_.size());

        // TODO: Rewrite to read blocks instead of one by one
        // Run until the MDC tag starts. 20 is hash length and 2 for the tag and the length
        const size_t kLeaveBytes = 20 + 2;
        static_assert(kCopyBufferSize >= kLeaveBytes, "Buffer size must be more than the size of the left bytes");
        Buffer copy_buf;
        copy_buf.resize(kCopyBufferSize);
        Buffer left_bytes;
        left_bytes.resize(kLeaveBytes);
        stream_length_type total_count = 0;
        while (!pipe.end_of_data())
        {
            auto bytes_read = pipe.read(copy_buf.begin(), kCopyBufferSize);
            total_count += bytes_read;

            // The complexity below is for leaving last 22 bytes without knowing the length of the pipe
            if(bytes_read >= kLeaveBytes)
            {
                if(total_count > static_cast<stream_length_type>(bytes_read)) // not first iteration
                {
                    // previous left bytes
                    hash.update(left_bytes.begin(), kLeaveBytes);
                    this->out_.Write(left_bytes.begin(), kLeaveBytes);
                }

                hash.update(copy_buf.begin(), bytes_read - kLeaveBytes);
                this->out_.Write(copy_buf.begin(), bytes_read - kLeaveBytes);

                // new left bytes
                std::copy_n(copy_buf.begin() + bytes_read - kLeaveBytes, 
                        kLeaveBytes, left_bytes.begin());
            }
            else
            {
                assert(pipe.end_of_data());
                hash.update(left_bytes.begin(), bytes_read);
                this->out_.Write(left_bytes.begin(), bytes_read);

                // this is a move to the left. Hopefully it handles overlapping ranges.
                std::copy_n(left_bytes.begin() + bytes_read, kLeaveBytes - bytes_read, left_bytes.begin());
                std::copy_n(copy_buf.begin(), bytes_read, left_bytes.begin() + kLeaveBytes - bytes_read);
            }
        }

        if(total_count < static_cast<stream_length_type>(kLeaveBytes))
            return PacketResult::UnexpectedFormat;

        const byte *ptr = left_bytes.begin();

        // tag and length of the MDC packet that need to be hashed too

        if(*ptr++ != 0xD3)
            return PacketResult::UnexpectedFormat;

        if(*ptr++ != 0x14)
            return PacketResult::UnexpectedFormat;

        hash.update(0xD3);
        hash.update(0x14);

        SecureVector<byte> actual_sha1 = hash.final();
        SecureVector<byte> expected_sha1;
        expected_sha1.resize(20);
        std::copy_n(ptr, 20, expected_sha1.begin());

        assert(ptr + 20 == left_bytes.end());

        if(actual_sha1 != expected_sha1)
            return PacketResult::MDCError;

        return PacketResult::Success;
    }

    PacketResult CompressedRW::DoRead()
    {
        byte b = in_.Get();
        Compression comp_algo = static_cast<Compression>(b);
        metadata_.compression = comp_algo;

        z_stream zs;
        memset(&zs, 0, sizeof(zs));
        int result = Z_OK;
        switch(comp_algo)
        {
            case Compression::ZIP:
                result = inflateInit2(&zs, -15); // -15 bits is for CryptoEX compatibility
                break;
            case Compression::ZLIB:
                result = inflateInit(&zs);
                break;
            default:
                //unsupported
                return PacketResult::UnsupportedCompressionAlgo;
        }

        std::array<byte, kCompressionBufferSize> buffer_in;
        std::array<byte, kCompressionBufferSize> buffer_out;

#ifndef NDEBUG 
        stream_length_type debug_test_counter = out_.GetCount();
#endif

        while(!in_.IsEOF())
        {
            int bytes_read = in_.Read(buffer_in.begin(), buffer_in.size());
            zs.next_in = buffer_in.begin();
            zs.avail_in = bytes_read;
            do
            {
                zs.next_out = buffer_out.begin();
                zs.avail_out = buffer_out.size();
                result = inflate(&zs, Z_SYNC_FLUSH);
                switch(result)
                {
                    case Z_OK:
                    case Z_STREAM_END:
                    case Z_BUF_ERROR:
                        break;
                    default:
                        return PacketResult::CompressionError;
                }
                out_.Write(buffer_out.begin(), buffer_out.size() - zs.avail_out);
            }
            while(result == Z_OK && !zs.avail_out);
        }

        assert(result == Z_STREAM_END);
        assert(zs.avail_in == 0);
        result = inflateEnd(&zs);

        if(result != Z_OK)
            return PacketResult::CompressionError;

        assert(out_.GetCount() - debug_test_counter == zs.total_out); 
        return PacketResult::Success;
    }

    PacketResult LiteralRW::DoRead()
    {
        byte b = in_.Get();
        switch(b)
        {
            // b binary, t text, u UTF8, l local, 1 local too
            case 'b': case 't': case 'u': case 'l': case '1':
                break;
            default:
                return PacketResult::UnexpectedFormat;
        }

        // it is the only important thing for us for now
        metadata_.is_binary = b == 'b';
        byte file_name_length = in_.Get();
        Buffer buffer;
        buffer.resize(file_name_length);
        auto read_bytes = in_.Read(buffer.begin(), file_name_length);
        if(read_bytes != file_name_length)
            return PacketResult::UnexpectedFormat;
        buffer.push_back('\0');
        metadata_.file_name = reinterpret_cast<const char*>(buffer.begin());
        // File Date
        buffer.resize(4);
        read_bytes = in_.Read(buffer.begin(), 4);
        if(read_bytes != 4)
            return PacketResult::UnexpectedFormat;

        static_assert(sizeof(metadata_.file_date) >= 4, "file_data_ type must be 4 bytes or more");

        memcpy(&metadata_.file_date, buffer.begin(), 4);

        Buffer copy_buf;
        copy_buf.resize(kCopyBufferSize);

        while(!in_.IsEOF())
        {
            auto bytes_read = in_.Read(copy_buf.begin(), kCopyBufferSize);
            out_.Write(copy_buf.begin(), bytes_read);
        }

        return PacketResult::Success;
    }
    // end of packet methods implementations
}
