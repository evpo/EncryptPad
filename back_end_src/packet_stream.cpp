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
#include "packet_stream.h"

namespace EncryptPad
{
    std::unique_ptr<OutPacketStreamCont> MakeOutStream(Botan::SecureVector<byte> &cnt)
    {
        std::unique_ptr<OutPacketStreamCont> ret_val( new OutPacketStreamCont(cnt));
        return ret_val;
    }

    stream_length_type ReadWithEOFHandler(InStream &stm, byte *out_it, stream_length_type bytes2read)
    {
        int bytes_read = 0;

        if(!stm.GetCount() && stm.GetEOFHandler())
            (*stm.GetEOFHandler())(stm);

        while(bytes2read && !stm.IsEOF())
        {
            stream_length_type step_length = std::min(stm.GetCount(), bytes2read);
            stream_length_type bytes = stm.ReadFromSource(out_it, step_length);
            bytes2read -= bytes;
            stm.SetCount(stm.GetCount() - bytes);
            assert(stm.GetCount() >= 0);
            bytes_read += bytes;
            out_it += bytes;

            if(!stm.GetCount() && stm.GetEOFHandler())
                (*stm.GetEOFHandler())(stm);
        }

        return bytes_read;
    }

    bool OutPacketStreamCont::DoPut(byte b)
    {
        assert(ptr_ <= out_->end());
        if(ptr_ == out_->end())
        {
            Resize(out_->size() + 1);
        }
        *ptr_++ = b;
        count_++;
        return true;
    }

    bool OutPacketStreamCont::DoWrite(const byte *in_it, stream_length_type bytes2write)
    {
        assert(ptr_ <= out_->end());
        assert(bytes2write >= 0);

        if(!bytes2write)
            return true;

        size_t free_bytes = out_->end() - ptr_;
        if(static_cast<stream_length_type>(free_bytes) < bytes2write)
        {
            Resize(out_->size() + bytes2write - free_bytes);
        }

        std::copy_n(in_it, bytes2write, ptr_);
        ptr_ += bytes2write;
        count_+=bytes2write;

        return true;
    }

    bool OutPacketStreamFile::DoPut(byte b)
    {
        if(fputc(b, file_.get()) == EOF)
        {
            return false;
        }

        count_ ++;

        return true;
    }

    bool OutPacketStreamFile::DoWrite(const byte *in_it, stream_length_type bytes2write)
    {
        if(!bytes2write)
            return true;

        assert(bytes2write > 0);

        size_t bytes = fwrite(in_it, sizeof(byte), bytes2write, file_.get());
        count_+=bytes;
        return static_cast<stream_length_type>(bytes) == bytes2write;
    }

    stream_length_type InPacketStreamPipe::DoRead(byte *out_it, stream_length_type bytes2read)
    {
        bytes2read = std::min(bytes2read, GetCount());
        int bytes_read = 0;

        while(bytes_read < bytes2read && !streams_.empty())
        {
            bytes_read += streams_.front()->Read(out_it, bytes2read - bytes_read);
            out_it += bytes_read;

            if(streams_.front()->IsEOF())
                streams_.pop();
        }

        count_ -= bytes_read;

        return bytes_read;
    }
}
