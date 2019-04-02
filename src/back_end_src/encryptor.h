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
#include "botan/secmem.h"
#include "key_service.h"
#include "packet_composer.h"
#include "epad_result.h"

namespace EncryptPadEncryptor
{

	class EncryptedPlainSwitchFunctor;

	class Encryptor
	{
	private:
        EncryptedPlainSwitchFunctor *mEncryptedPlainSwitchFunctor;
        EncryptPad::KeyService key_service_;
        EncryptPad::KeyService kf_key_service_;
        bool mPlainText;
        std::string mX2KeyLocation;
        std::string mLibcurlPath;
        std::string mLibcurlParams;
        Encryptor(const Encryptor &);
        Encryptor & operator=(const Encryptor &);
    public:
        Encryptor():mEncryptedPlainSwitchFunctor(nullptr),
            kf_key_service_(1), mPlainText(true){}

        EncryptPad::KeyService &GetKFKeyService()
        {
            return kf_key_service_;
        }

        EncryptPad::KeyService &GetKeyService()
        {
            return key_service_;
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
        void SetLibcurlParams(const std::string &params);

		// Saves content to fileName
		// persists the x2 key location in fileName if persistX2KeyLocation
        EncryptPad::EpadResult Save(const std::string &fileName, const Botan::SecureVector<Botan::byte> &content, 
                const std::string &x2KeyLocation = "", bool persistX2KeyLocation = false, 
                EncryptPad::PacketMetadata *metadata = nullptr, const std::string *kf_passphrase = nullptr
                );

        EncryptPad::EpadResult Load(const std::string &fileName, Botan::SecureVector<Botan::byte> &content, 
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

