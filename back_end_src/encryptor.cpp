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
#include "encryptor.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <cctype>
#include <regex>
#include "assert.h"
#include "file_system.hpp"
#include "os_api.h"
#include "encryption_pipeline.h"
#include "x2_key_loader.h"
#include "file_encryption.h"
#include "file_helper.h"

using namespace std;
using namespace EncryptPadEncryptor;
using namespace EncryptPad;
using namespace Botan;

namespace EncryptPadEncryptor
{
	const char *Encryptor::sSalt = "f74b42ab-19c9-49c9-b234-09cc730aa6e3";
	const char *x2KeyParameterName = "x2Key-A102D686-D935-4792-817E-D4E55C9ACF9F";
}

namespace
{
	bool ParseX2Key(Pipe &pipe, std::string &key)
	{
		string parameterName(x2KeyParameterName);
		size_t length2read = parameterName.length() + 1;
		unique_ptr<byte[]> buffer(new byte[length2read]);
		size_t length = pipe.peek(buffer.get(), length2read, 0);
		if (length != length2read)
			return false;

		string bufStr(reinterpret_cast<const char *>(buffer.get()), length2read);
		if ((parameterName + string("=")) != bufStr)
			return false;

		pipe.read(buffer.get(), length2read);

		byte b = 0x00;
		while (!pipe.end_of_data() && b != 10)
		{
			pipe.read_byte(b);
			if (b == 13 || b == 10)
				continue;
			key += static_cast<char>(b);
		}

		if (b != 10)
			return false;

		return true;
	}

    bool ConvertFromBase64(const std::string &base64_str, OctetString &out)
    {
		try
		{
			out = OctetString(base64_decode(base64_str));
		}
		catch (const std::invalid_argument &ex)
		{
            return false;
		}
        return true;
    }

    Result FromPacketResult(PacketResult result)
    {
        struct PacketResult2Result
        {
            PacketResult packet_result;
            Result result;
        };

        static PacketResult2Result table[] =
        {
            {PacketResult::Success, Result::OK},
            {PacketResult::IOErrorKeyFile, Result::X2KeyIOError},
            {PacketResult::InvalidKeyFile,  Result::InvalidX2File},
            {PacketResult::CurlExitNonZero,  Result::X2CurlExitNonZero},
            {PacketResult::CurlIsNotFound,  Result::X2CurlIsNotFound},
            {PacketResult::UnexpectedError, Result::CpadFileIOError}, 
            {PacketResult::InvalidSurrogateIV, Result::EncryptionError}, 
            {PacketResult::Empty, Result::None}, 
            {PacketResult::UnexpectedFormat, Result::InvalidCpadFile}, 
            {PacketResult::UnsupportedPacketType, Result::InvalidCpadFile}, 
            {PacketResult::UnsupportedAlgo, Result::InvalidCpadFile}, 
            {PacketResult::UnsupportedS2K, Result::InvalidCpadFile}, 
            {PacketResult::UnsupportedCompressionAlgo, Result::InvalidCpadFile}, 
            {PacketResult::IOError, Result::CpadFileIOError}, 
            {PacketResult::IOErrorInput, Result::CpadFileIOError}, 
            {PacketResult::IOErrorOutput, Result::CpadFileIOError}, 
            {PacketResult::MDCError, Result::InvalidCpadFile}, 
            {PacketResult::CompressionError, Result::InvalidCpadFile}, 
            {PacketResult::KeyFileNotSpecified, Result::X2FileIsRequired}, 
            {PacketResult::InvalidWadFile, Result::InvalidCpadFile}, 
            {PacketResult::InvalidPassphrase, Result::EncryptionError}, 
            {PacketResult::KeyIsRequiredForSaving, Result::EncryptionError}, 
            {PacketResult::InvalidKeyFilePassphrase, Result::InvalidKeyFilePassphrase},
            {PacketResult::None, Result::None},
        };

        PacketResult2Result *p = table;
        while(p->packet_result != result && p->packet_result != PacketResult::None)
        {
            ++p;
        }

        if(p->packet_result == PacketResult::None)
            assert(false);
        return p->result;
    }

    bool IsCpadFormat(const std::string &file_name)
    {
        auto ext = stlplus::extension_part(file_name);
        return ext == "cpad";
    }
}

void Encryptor::SetLibcurlPath(const std::string &path)
{
    mLibcurlPath = path;
}

const std::string &Encryptor::GetLibcurlPath() const
{
    return mLibcurlPath;
}

void Encryptor::SetEncryptedPlainSwitchFunctor(EncryptedPlainSwitchFunctor *functor)
{
    mEncryptedPlainSwitchFunctor = functor;
}

void Encryptor::ClearKFPassphrase()
{
    kf_key_service_.Clear();
}

bool Encryptor::HasKFPassphrase() const
{
    return kf_key_service_.IsPassphraseSet();
}

void Encryptor::SetIsPlainText()
{
    key_service_.Clear();
	mPlainText = true;
	OctetString emptyKey, emptyIV;
	mKey = emptyKey;
	mIV = emptyIV;
	if (mEncryptedPlainSwitchFunctor)
		mEncryptedPlainSwitchFunctor->EncryptedPlainSwitchChange(false);
}

bool Encryptor::GetIsPlainText()
{
    return this->mPlainText;
}

bool Encryptor::UnusedKeysExist()
{
    return key_service_.UnusedKeysExist();
}

const string &Encryptor::GetX2KeyLocation() const
{
	return this->mX2KeyLocation;
}

void Encryptor::SetPassphrase(const char *pwd, EncryptPad::PacketMetadata *metadata)
{

    key_service_.Clear();
    if(metadata)
    {
        std::string passphrase(pwd);
        key_service_.ChangePassphrase(
                passphrase, 
                metadata->hash_algo, 
                GetAlgoSpec(metadata->cipher_algo).key_size);
        std::fill(std::begin(passphrase), std::end(passphrase), '0');

        // if there was key_only before, switch it off
        metadata->key_only = false;
    }

	PKCS5_PBKDF2* pbkdf = static_cast<PKCS5_PBKDF2 *>(get_s2k("PBKDF2(SHA-1)"));

	const byte *salt = (byte *)(Encryptor::sSalt);

	OctetString buf = pbkdf->derive_key(
		sKeySize / 8 + sBlockSize / 8,
		pwd, salt, strlen(Encryptor::sSalt), Encryptor::sIterationCount);
	OctetString key(buf.begin(), sKeySize / 8);
	OctetString iv(buf.begin() + sKeySize / 8, sBlockSize / 8);
	mKey = key;
	mIV = iv;
	mPlainText = false;

	if (mEncryptedPlainSwitchFunctor)
		mEncryptedPlainSwitchFunctor->EncryptedPlainSwitchChange(true);
}

EncryptPadEncryptor::Result Encryptor::Save(const string &fileName, const SecureVector<byte> &content, 
        const string &x2KeyLocation, bool persistX2KeyLocation, 
        PacketMetadata *metadata, const std::string *kf_passphrase)
{
    bool is_protected = !GetIsPlainText() || !x2KeyLocation.empty();

    if(!IsCpadFormat(fileName) && is_protected)
    {
        // New EncryptPad format
        using namespace EncryptPad;

        assert(metadata);

        mX2KeyLocation = x2KeyLocation;
        metadata->key_file = mX2KeyLocation;

        if(GetIsPlainText())
            metadata->key_only = true;

        metadata->persist_key_path = persistX2KeyLocation;

        EncryptParams enc_params = {};
        enc_params.key_service = &key_service_;
        enc_params.libcurl_path = &mLibcurlPath;

        EncryptParams kf_encrypt_params = {};

        if(kf_passphrase || kf_key_service_.IsPassphraseSet())
        {
            kf_encrypt_params.key_service = &kf_key_service_;
            kf_encrypt_params.passphrase = kf_passphrase;
            enc_params.key_file_encrypt_params = &kf_encrypt_params;
        }

        auto packetResult = EncryptPacketFile(content, fileName, enc_params, *metadata);
        auto result = FromPacketResult(packetResult); 

        if(result != Result::OK)
            return result;
        if(metadata->key_only)
            this->SetIsPlainText();

        return result;
    }

	assert(!(x2KeyLocation.empty() && persistX2KeyLocation));

    std::string x2Key;
	Result loadKeyResult = Result::OK;

	if (!x2KeyLocation.empty() && (loadKeyResult = FromPacketResult(LoadKeyFromFile(x2KeyLocation, mLibcurlPath, x2Key))) != Result::OK)
    {
		return loadKeyResult;
    }

    OctetString octet_key;
    if(!ConvertFromBase64(x2Key, octet_key))
        return Result::InvalidX2File;

    OutPacketStreamFile file;

	if (OpenFile(fileName, file) != OpenFileResult::OK)
	{
		return Result::CpadFileIOError;
	}

	Pipe firstStepTgtPipe;

	SecureVector<byte> cpad_metadata;
	if(!x2KeyLocation.empty())
	{
		string metadataString = x2KeyParameterName + string("=") + (persistX2KeyLocation ? x2KeyLocation : string(""));
		cpad_metadata.resize(metadataString.size());
		cpad_metadata.copy(reinterpret_cast<const byte*>(metadataString.c_str()), metadataString.size());
		cpad_metadata.push_back(13);
		cpad_metadata.push_back(10);

		try
		{
			ConvertToPipe(content, SecureVector<byte>(), firstStepTgtPipe, ENCRYPTION, octet_key, mIV, "AES-256/CBC");
		}
		catch (const Botan::Invalid_Block_Size)
		{
			return Result::InvalidX2File;
		}
		catch (const Botan::Invalid_Key_Length)
		{
			return Result::InvalidX2File;
		}
	}

	OctetString emptyOctet;
	const OctetString *key = mPlainText ? &emptyOctet : &mKey;
	const OctetString *iv = mPlainText ? &emptyOctet : &mIV;

	Pipe pipe;

	if (!x2KeyLocation.empty())
	{
		ConvertToPipe(firstStepTgtPipe, cpad_metadata, pipe, ENCRYPTION, *key, *iv, "AES-256/CBC");
	}
	else
	{
		ConvertToPipe(content, cpad_metadata, pipe, ENCRYPTION, *key, *iv, "AES-256/CBC");
	}

    Botan::u32bit read_bytes;
    byte buffer[kBufferSize];
    while(!pipe.end_of_data())
    {
        read_bytes = pipe.read(buffer, kBufferSize);
        file.Write(buffer, read_bytes);
    }

	return Result::OK;
}

// Loads file, decrypts into content
Result Encryptor::Load(const std::string &fileName, SecureVector<byte> &content, 
        const string &x2KeyLocation, const string *passphrase, 
        EncryptPad::PacketMetadata *metadata, const string *kf_passphrase)
{
    bool is_protected = !GetIsPlainText() || !x2KeyLocation.empty();
    bool is_cpad = IsCpadFormat(fileName);
    if(!is_cpad && is_protected)
    {
        assert(metadata);
        // New EncryptPad format
        using namespace EncryptPad;
        mX2KeyLocation = x2KeyLocation;
        metadata->key_file = mX2KeyLocation;

        EncryptParams enc_params = {};
        enc_params.passphrase = passphrase;
        enc_params.key_service = &key_service_;
        enc_params.libcurl_path = &mLibcurlPath;

        EncryptParams kf_encrypt_params = {};

        if(kf_passphrase || kf_key_service_.IsPassphraseSet())
        {
            kf_encrypt_params.key_service = &kf_key_service_;
            kf_encrypt_params.passphrase = kf_passphrase;
            enc_params.key_file_encrypt_params = &kf_encrypt_params;
        }

        auto result = FromPacketResult(DecryptPacketFile(fileName, enc_params, content, *metadata)); 
        if(result != Result::OK)
            return result;
        mX2KeyLocation = metadata->key_file;
        if(metadata->key_only)
            this->SetIsPlainText();

        return result;
    }

    InPacketStreamFile file;

	if (OpenFile(fileName, file) != OpenFileResult::OK)
	{
		return Result::CpadFileIOError;
	}

    // For unencrypted text
    if(mKey.length() == 0 && mX2KeyLocation.empty() && x2KeyLocation.empty() && !is_cpad)
    {
        content.resize(file.GetCount());
        file.Read(content.begin(), file.GetCount());
        return Result::OK;
    }

	// phase 1 decrypt with the passphrase
	Pipe pipe;

	try
	{
		ConvertToPipe(file, SecureVector<byte>(), pipe, DECRYPTION, mKey, mIV, "AES-256/CBC");
	}
	catch (const Botan::Decoding_Error &)
	{
		return Result::EncryptionError;
	}

	// phase 2 decrypt with the key
	mX2KeyLocation = x2KeyLocation;
	bool parseResult = false;
	// x2key location parsed out to empty string and disposed if the location is provided
	// If not parsed out, the plain text x2key location will break the decryption below.
	if (mX2KeyLocation.empty())
	{
		parseResult = ParseX2Key(pipe, mX2KeyLocation);
		// Special case: the file is x2 encrypted but the key is not persistent and not provided
		if(parseResult && mX2KeyLocation.empty())
			return Result::X2FileIsRequired;
	}
	else
	{
		string tmp;
		parseResult = ParseX2Key(pipe, tmp);
		if(!parseResult)
			mX2KeyLocation.clear();
	}

    std::string x2Key;
	Result loadKeyResult = Result::OK;

	if (!mX2KeyLocation.empty() && (loadKeyResult = FromPacketResult(LoadKeyFromFile(mX2KeyLocation, mLibcurlPath, x2Key))) != Result::OK)
	{
		return loadKeyResult;
	}

    OctetString octet_key;
    if(!ConvertFromBase64(x2Key, octet_key))
        return Result::InvalidX2File;

	Pipe finalPipe;
	try
	{
		if (!mX2KeyLocation.empty())
			ConvertToPipe(pipe, SecureVector<byte>(), finalPipe, DECRYPTION, octet_key, mIV, "AES-256/CBC");
	}
	catch (const Botan::Decoding_Error &)
	{
		return Result::EncryptionError;
	}
	catch (const Botan::Invalid_Block_Size &)
	{
		return Result::InvalidX2File;
	}
	catch (const Botan::Invalid_Key_Length &)
	{
		return Result::InvalidX2File;
	}

	Pipe *pipe2Load = mX2KeyLocation.empty() ? &pipe : &finalPipe;

    auto out = MakeOutStream(content);

    Botan::u32bit read_bytes;
    byte buffer[kBufferSize];
	while (!pipe2Load->end_of_data())
	{
		read_bytes = pipe2Load->read(buffer, kBufferSize);
        out->Write(buffer, read_bytes);
	}

    content.resize(out->GetCount());

	return Result::OK;
}

