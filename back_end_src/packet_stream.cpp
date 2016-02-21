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
}
