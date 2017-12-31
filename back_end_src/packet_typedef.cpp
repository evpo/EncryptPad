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
#include "packet_typedef.h"
#include "assert.h"

using namespace LibEncryptMsg;

namespace EncryptPad
{
    std::string InterpretResult(LibEncryptMsg::PacketResult result)
    {
        switch(result)
        {
            case PacketResult::UnexpectedError:
                return "Unexpected error";
            case PacketResult::InvalidSurrogateIV:
                return "Incorrect passphrase";
            case PacketResult::Empty:
                return "Composite packet is empty";
            case PacketResult::UnexpectedFormat:
                return "Unexpected format";
            case PacketResult::UnsupportedPacketType:
                return "Packet type is not supported";
            case PacketResult::UnsupportedAlgo:
                return "Encryption algorithm is not supported";
            case PacketResult::UnsupportedS2K:
                return "S2K algorithm is not supported";
            case PacketResult::UnsupportedCompressionAlgo:
                return "Unsupported compression algorithm";
            case PacketResult::IOError:
                return "Input / Output error";
            case PacketResult::IOErrorInput:
                return "Input error";
            case PacketResult::IOErrorOutput:
                return "Output error";
            case PacketResult::MDCError:
                return "File integrity check failed"; 
            case PacketResult::CompressionError:
                return "Compression / decompression error";
            case PacketResult::KeyFileNotSpecified:
                return "Key file is required. Use -k option.";
            case PacketResult::IOErrorKeyFile:
                return "Cannot read key file";
            case PacketResult::InvalidKeyFile:
                return "Invalid key file";
            case PacketResult::CurlIsNotFound:
                return "Cannot get key file: CURL executable is not found";
            case PacketResult::CurlExitNonZero:
                return "Cannot get key file: CURL returned non-zero exit code";
            case PacketResult::InvalidWadFile:
                return "Unexpected format: invalid wad file";
            case PacketResult::InvalidOrIncompleteWadFile:
                return "Unexpected format: invalid or incomplete wad file";
            case PacketResult::InvalidPassphrase:
                return "Key service does not containt a suitable key";
            case PacketResult::KeyIsRequiredForSaving:
                return "Key file is required. Use -k option.";
            case PacketResult::InvalidKeyFilePassphrase:
                return "Key file passphrase is invalid";
            default:
                assert(false);
                return "";
        }
    }
}
