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
#ifndef FILE_ENCRYPTION_DLG_ADAPTER_H
#define FILE_ENCRYPTION_DLG_ADAPTER_H

#include "load_save_handler.h"

class FileEncryptionDialog;

class FileEncryptionDlgAdapter : public EncryptPad::LoadHandlerClient
{
private:
    FileEncryptionDialog *parent;
public:
    FileEncryptionDlgAdapter(FileEncryptionDialog *parent);
    virtual bool IsPassphraseNotSet() const override;
    virtual void SetIsPlainText() override;
    virtual void SetPassphrase(const char *pwd, EncryptPad::PacketMetadata &metadata) override;
    virtual QString EncryptionKeyFile() const override;
    virtual void EncryptionKeyFile(const QString &keyFile) override;
    virtual bool PersistEncryptionKeyPath() const override;
    virtual void PersistEncryptionKeyPath(bool flag) override;
    virtual bool HasKeyFilePassphrase() const override;
    virtual void UpdateEncryptionKeyStatus() override;
    virtual FileRequestService &GetFileRequestService() override;
    virtual void StartLoad(const QString &fileName, const QString &encryptionKeyFile,
            std::string &passphrase, EncryptPad::PacketMetadata &metadata, std::string &kf_passphrase) override;
    virtual void StartSave(const QString &fileName, std::string &kf_passphrase) override;
};

#endif // FILE_ENCRYPTION_DLG_ADAPTER_H
