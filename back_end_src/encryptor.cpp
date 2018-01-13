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
#include <string>
#include "file_encryption.h"
#include "file_helper.h"

using namespace std;
using namespace EncryptPadEncryptor;
using namespace EncryptPad;
using namespace Botan;

const size_t kBufferSize = 1024;

void Encryptor::SetLibcurlParams(const std::string &params)
{
    mLibcurlParams = params;
}

void Encryptor::SetLibcurlPath(const std::string &path)
{
    mLibcurlPath = path;
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
    assert(metadata);
    key_service_.Clear();
    if(metadata)
    {
        std::string passphrase(pwd);
        key_service_.ChangePassphrase(
                passphrase, 
                metadata->hash_algo, 
                GetAlgoSpec(metadata->cipher_algo).key_size,
                metadata->iterations);
        std::fill(std::begin(passphrase), std::end(passphrase), '0');

        // if there was key_only before, switch it off
        metadata->key_only = false;
    }

	mPlainText = false;

	if (mEncryptedPlainSwitchFunctor)
		mEncryptedPlainSwitchFunctor->EncryptedPlainSwitchChange(true);
}

EpadResult Encryptor::Save(const string &fileName, const SecureVector<byte> &content, 
        const string &x2KeyLocation, bool persistX2KeyLocation,
        PacketMetadata *metadata, const std::string *kf_passphrase)
{
    using namespace EncryptPad;

    bool is_protected = !GetIsPlainText() || !x2KeyLocation.empty();

    if(!is_protected)
    {
        OutPacketStreamFile file;
        if (OpenFile(fileName, file) != OpenFileResult::OK)
        {
            return EpadResult::IOErrorOutput;
        }

        if(!file.Write(content.data(), content.size()))
        {
            return EpadResult::IOErrorOutput;
        }

        return EpadResult::Success;
    }

    assert(metadata);
    mX2KeyLocation = x2KeyLocation;
    metadata->key_file = mX2KeyLocation;

    if(GetIsPlainText())
        metadata->key_only = true;

    metadata->persist_key_path = persistX2KeyLocation;

    EncryptParams enc_params = {};
    enc_params.key_service = &key_service_;
    enc_params.libcurl_path = &mLibcurlPath;
    enc_params.libcurl_parameters = &mLibcurlParams;

    EncryptParams kf_encrypt_params = {};

    if(kf_passphrase || kf_key_service_.IsPassphraseSet())
    {
        kf_encrypt_params.key_service = &kf_key_service_;
        kf_encrypt_params.passphrase = kf_passphrase;
        enc_params.key_file_encrypt_params = &kf_encrypt_params;
    }

    EpadResult result = EncryptPacketFile(content, fileName, enc_params, *metadata);

    if(result != EpadResult::Success)
        return result;
    if(metadata->key_only)
        this->SetIsPlainText();

    return result;
}

// Loads file, decrypts into content
EpadResult Encryptor::Load(const std::string &fileName, SecureVector<byte> &content, 
        const string &x2KeyLocation, const string *passphrase, 
        EncryptPad::PacketMetadata *metadata, const string *kf_passphrase)
{
    using namespace EncryptPad;

    bool is_protected = !GetIsPlainText() || !x2KeyLocation.empty();
    if(!is_protected)
    {
        InPacketStreamFile file;
        if (OpenFile(fileName, file) != OpenFileResult::OK)
        {
            return EpadResult::IOErrorInput;
        }

        content.resize(file.GetCount());
        stream_length_type length = file.Read(content.data(), content.size());
        if(length != static_cast<stream_length_type>(content.size()))
            return EpadResult::IOErrorInput;

        return EpadResult::Success;
    }

    assert(metadata);
    mX2KeyLocation = x2KeyLocation;
    metadata->key_file = mX2KeyLocation;

    EncryptParams enc_params = {};
    enc_params.passphrase = passphrase;
    enc_params.key_service = &key_service_;
    enc_params.libcurl_path = &mLibcurlPath;
    enc_params.libcurl_parameters = &mLibcurlParams;

    EncryptParams kf_encrypt_params = {};

    if(kf_passphrase || kf_key_service_.IsPassphraseSet())
    {
        kf_encrypt_params.key_service = &kf_key_service_;
        kf_encrypt_params.passphrase = kf_passphrase;
        enc_params.key_file_encrypt_params = &kf_encrypt_params;
    }

    EpadResult result = DecryptPacketFile(fileName, enc_params, content, *metadata); 
    if(result != EpadResult::Success)
        return result;
    mX2KeyLocation = metadata->key_file;
    if(metadata->key_only)
        this->SetIsPlainText();

    return result;
}

