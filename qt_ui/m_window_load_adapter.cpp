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
#include "m_window_load_adapter.h"
#include "mainwindow.h"

    bool LoadHandlerAdapter::IsPassphraseNotSet() const
    {
        return parent->enc.GetIsPlainText() || !parent->enc.UnusedKeysExist();
    }

    void LoadHandlerAdapter::SetIsPlainText()
    {
        parent->enc.SetIsPlainText();
    }

    void LoadHandlerAdapter::SetPassword(const char *pwd, EncryptPad::PacketMetadata &metadata)
    {
        parent->enc.SetPassword(pwd, &metadata);
    }

    QString LoadHandlerAdapter::EncryptionKeyFile() const
    {
        return parent->getEncryptionKeyFile();
    }

    void LoadHandlerAdapter::EncryptionKeyFile(const QString &keyFile)
    {
        parent->setEncryptionKeyFile(keyFile);
    }

    bool LoadHandlerAdapter::PersistEncryptionKeyPath() const
    {
        return parent->persistEncryptionKeyPath;
    }

    void LoadHandlerAdapter::PersistEncryptionKeyPath(bool flag)
    {
        parent->persistEncryptionKeyPath = flag;
    }

    bool LoadHandlerAdapter::HasKeyFilePassphrase() const
    {
        return parent->enc.HasKFPassphrase();
    }

    void LoadHandlerAdapter::UpdateEncryptionKeyStatus()
    {
        parent->updateEncryptionKeyStatus();
    }

    FileRequestService &LoadHandlerAdapter::GetFileRequestService()
    {
        return parent->file_request_service_;
    }

    void LoadHandlerAdapter::StartLoad(const QString &fileName, const QString &encryptionKeyFile,
            std::string &passphrase, EncryptPad::PacketMetadata &metadata, std::string &kf_passphrase)
    {
        parent->startLoad(fileName, encryptionKeyFile, passphrase, metadata, kf_passphrase);
    }

    void LoadHandlerAdapter::StartSave(const QString &fileName, std::string &kf_passphrase)
    {
        parent->startSave(fileName, kf_passphrase);
    }
