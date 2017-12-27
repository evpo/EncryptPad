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
#pragma once
#include <fstream>
#include "botan/botan.h"
#include "packet_stream.h"

namespace EncryptPad
{
	const size_t kBufferSize = 1024;
	void WriteToPipe(std::ifstream &src, Botan::Pipe &dest);

	void WriteToPipe(Botan::SecureVector<byte> &src, Botan::Pipe &dest);
	void WriteToPipe(const Botan::SecureVector<byte> &src, Botan::Pipe &dest);

	void WriteToPipe(Botan::Pipe &src, Botan::Pipe &dest);

    void WriteToPipe(InStream &src, Botan::Pipe &dest);

	template<typename T>
	void ConvertToPipe(T &source, const Botan::SecureVector<byte> &metadata, Botan::Pipe &tgtPipe)
	{
		tgtPipe.start_msg();
		tgtPipe.write(metadata);
		WriteToPipe(source, tgtPipe);
		tgtPipe.end_msg();
	}

	template<typename T>
	void ConvertToPipe(T &source, const Botan::SecureVector<byte> &metadata, Botan::Pipe &tgtPipe, Botan::Cipher_Dir direction,
		const Botan::OctetString &key = Botan::OctetString(), const Botan::OctetString &iv = Botan::OctetString(),
        const std::string cipher_algo_name = "AES-256/CFB")
	{
		if (key.length() > 0)
			tgtPipe.append(Botan::get_cipher(cipher_algo_name, key, iv, direction));

        ConvertToPipe(source, metadata, tgtPipe);
	}


}
