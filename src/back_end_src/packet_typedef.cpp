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

using namespace EncryptMsg;

namespace EncryptPad
{
    std::string InterpretResult(EpadResult result)
    {
        switch(result)
        {
            case EpadResult::UnexpectedError:
                return "Unexpected error";
            case EpadResult::InvalidSurrogateIV:
                return "Incorrect passphrase";
            case EpadResult::Empty:
                return "Composite packet is empty";
            case EpadResult::UnexpectedFormat:
                return "Unexpected format";
            case EpadResult::UnsupportedPacketType:
                return "Packet type is not supported";
            case EpadResult::UnsupportedAlgo:
                return "Encryption algorithm is not supported";
            case EpadResult::UnsupportedS2K:
                return "S2K algorithm is not supported";
            case EpadResult::UnsupportedCompressionAlgo:
                return "Unsupported compression algorithm";
            case EpadResult::IOError:
                return "Input / Output error";
            case EpadResult::IOErrorInput:
                return "Input error";
            case EpadResult::IOErrorOutput:
                return "Output error";
            case EpadResult::MDCError:
                return "File integrity check failed"; 
            case EpadResult::CompressionError:
                return "Compression / decompression error";
            case EpadResult::KeyFileNotSpecified:
                return "Key file is required. Use -k option.";
            case EpadResult::IOErrorKeyFile:
                return "Cannot read key file";
            case EpadResult::InvalidKeyFile:
                return "Invalid key file";
            case EpadResult::CurlIsNotFound:
                return "Cannot get key file: CURL executable is not found";
            case EpadResult::CurlExitNonZero:
                return "Cannot get key file: CURL returned non-zero exit code";
            case EpadResult::InvalidWadFile:
                return "Unexpected format: invalid wad file";
            case EpadResult::InvalidOrIncompleteWadFile:
                return "Unexpected format: invalid or incomplete wad file";
            case EpadResult::InvalidPassphrase:
                return "Key service does not containt a suitable key";
            case EpadResult::KeyIsRequiredForSaving:
                return "Key file is required. Use -k option.";
            case EpadResult::InvalidKeyFilePassphrase:
                return "Key file passphrase is invalid";
            case EpadResult::Cancelled:
                return "Cancelled by the user";
            default:
                assert(false);
                return "";
        }
    }
}
