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

#include <string>
#include "botan.h"
#include "key_service.h"
#include "packet_composer.h"

namespace EncryptPadEncryptor
{

	class EncryptedPlainSwitchFunctor;

    enum Result
	{
		OK = 0,
		EncryptionError,
		X2KeyIOError,
		InvalidX2File,
		X2FileIsRequired,
        CpadFileIOError,
        InvalidCpadFile,
        X2CurlIsNotFound,
        X2CurlExitNonZero,
        InvalidKeyFilePassphrase,
        BakFileMoveFailed,
        None,
	};

	class Encryptor
	{
	private:
		static const char* sSalt;
		static const char* sIV4X2;
		static const int sKeySize = 256;
		static const int sBlockSize = 128;
		static const int sBufferSizeForReading = 256;
		static const int sIterationCount = 1000;

        Botan::LibraryInitializer mInit;

        EncryptedPlainSwitchFunctor *mEncryptedPlainSwitchFunctor;
        EncryptPad::KeyService key_service_;
        EncryptPad::KeyService kf_key_service_;
        bool mPlainText;
        Botan::OctetString mKey;
        Botan::OctetString mIV;
        std::string mX2KeyLocation;
        std::string mLibcurlPath;
        Encryptor(const Encryptor &);
        Encryptor & operator=(const Encryptor &);
    public:
        Encryptor():mEncryptedPlainSwitchFunctor(NULL),
            kf_key_service_(1), mPlainText(true){}

        EncryptPad::KeyService &GetKFKeyService()
        {
            return kf_key_service_;
        }

		void SetPassphrase(const char *pwd, EncryptPad::PacketMetadata *metadata = nullptr);
		void SetIsPlainText();
		bool GetIsPlainText();
        bool UnusedKeysExist();
        const std::string &GetX2KeyLocation() const;
		void SetEncryptedPlainSwitchFunctor(EncryptedPlainSwitchFunctor *functor);
        void ClearKFPassphrase();
        bool HasKFPassphrase() const;
        void SetLibcurlPath(const std::string &path);
        const std::string &GetLibcurlPath() const;

		// Saves content to fileName
		// persists the x2 key location in fileName if persistX2KeyLocation
		Result Save(const std::string &fileName, const Botan::SecureVector<Botan::byte> &content, 
                const std::string &x2KeyLocation = "", bool persistX2KeyLocation = false, 
                EncryptPad::PacketMetadata *metadata = nullptr, const std::string *kf_passphrase = nullptr
                );

		Result Load(const std::string &fileName, Botan::SecureVector<Botan::byte> &content, 
                const std::string &x2KeyLocation = "", const std::string *passphrase = nullptr, 
                EncryptPad::PacketMetadata *metadata = nullptr, const std::string *kf_passphrase = nullptr
                );

	};

	class EncryptedPlainSwitchFunctor
	{
	public:
		EncryptedPlainSwitchFunctor()
		{
		}

		virtual ~EncryptedPlainSwitchFunctor()
		{
		}

		virtual void EncryptedPlainSwitchChange(bool encrypted) = 0;
	private:
		EncryptedPlainSwitchFunctor(const EncryptedPlainSwitchFunctor &);
		EncryptedPlainSwitchFunctor & operator=(const EncryptedPlainSwitchFunctor &);
	};

}

