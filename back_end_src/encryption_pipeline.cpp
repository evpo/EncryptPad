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
#include "encryption_pipeline.h"

namespace EncryptPad
{
	void WriteToPipe(std::ifstream &src, Botan::Pipe &dest)
	{
		char buf[kBufferSize];
		while (src.good())
		{
			src.read(buf, kBufferSize);
			const byte *byteArr = reinterpret_cast<const byte *>(buf);
			auto lastReadCount = static_cast<size_t>(src.gcount());
			dest.write(byteArr, std::min(lastReadCount, kBufferSize));
		}
	}

	void WriteToPipe(const Botan::SecureVector<byte> &src, Botan::Pipe &dest)
	{
		dest.write(src);
	}

	void WriteToPipe(Botan::SecureVector<byte> &src, Botan::Pipe &dest)
	{
        const Botan::SecureVector<byte> &v = src;
        WriteToPipe(v, dest);
	}

    void WriteToPipe(InStream &src, Botan::Pipe &dest)
    {
        byte buffer[kBufferSize];
		while (!src.IsEOF())
		{
			Botan::u32bit bytesRead = src.Read(buffer, kBufferSize);
			dest.write(buffer, bytesRead);
		}
    }

	void WriteToPipe(Botan::Pipe &src, Botan::Pipe &dest)
	{
		while (!src.end_of_data())
		{
			byte buffer[kBufferSize];
			Botan::u32bit bytesRead = src.read(buffer, kBufferSize);
			dest.write(buffer, bytesRead);
		}
	}
}
