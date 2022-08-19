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
#include <QtWidgets>
#include <QString>
#include "packet_composer.h"
#include "async_load.h"
#include "file_request_service.h"

namespace EncryptPad
{
    class LoadHandlerClient
    {
    public:
        virtual bool IsPassphraseNotSet() const = 0;
        virtual void SetIsPlainText() = 0;
        virtual void SetPassphrase(const char *pwd, EncryptPad::PacketMetadata &metadata) = 0;
        virtual QString EncryptionKeyFile() const = 0;
        virtual void EncryptionKeyFile(const QString &keyFile) = 0;
        virtual bool HasKeyFilePassphrase() const = 0;
        virtual bool PersistEncryptionKeyPath() const = 0;
        virtual void PersistEncryptionKeyPath(bool flag) = 0;
        virtual void UpdateEncryptionKeyStatus() = 0;
        virtual FileRequestService &GetFileRequestService() = 0;
        virtual void StartLoad(const QString &fileName, const QString &encryptionKeyFile,
                std::string &passphrase, EncryptPad::PacketMetadata &metadata, std::string &kf_passphrase) = 0;
        virtual void StartSave(const QString &fileName, std::string &kf_passphrase) = 0;
    };

    class LoadHandler
    {
    private:
        QWidget *parent;
        LoadHandlerClient &client;
        PacketMetadata &metadata;

    public:
        LoadHandler(QWidget *p, LoadHandlerClient &c, PacketMetadata &m)
            :parent(p), client(c), metadata(m)
        {
        }

        bool OpenPassphraseDialog(bool confirmationEnabled, std::string *passphrase = nullptr, bool set_client_passphrase = true, const QString &title = QString());
        bool OpenSetEncryptionKeyDialogue();

        // returns true if StartLoad was called
        bool LoadFile(const QString &fileName, bool force_kf_passphrase_request = false);

        // returns true if StartSave was called
        bool SaveFile(const QString &fileName, bool allow_unencrypted = true);
    };
}
