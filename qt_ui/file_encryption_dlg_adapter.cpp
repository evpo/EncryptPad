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
#include "file_encryption_dlg_adapter.h"
#include "file_encryption_dialog.h"

FileEncryptionDlgAdapter::FileEncryptionDlgAdapter(FileEncryptionDialog *p)
  :parent(p)
{
}

bool FileEncryptionDlgAdapter::IsPassphraseNotSet() const
{
    return parent->IsPassphraseNotSet();
}

void FileEncryptionDlgAdapter::SetIsPlainText()
{
    parent->ClearPassphrase();
}

void FileEncryptionDlgAdapter::SetPassphrase(const char *pwd, EncryptPad::PacketMetadata &metadata)
{
    parent->SetPassphrase(pwd, metadata);
}

QString FileEncryptionDlgAdapter::EncryptionKeyFile() const
{
    return parent->EncryptionFilePath();
}

void FileEncryptionDlgAdapter::EncryptionKeyFile(const QString &keyFile)
{
    parent->EncryptionFilePath(keyFile);
}

bool FileEncryptionDlgAdapter::PersistEncryptionKeyPath() const
{
    return parent->PersistKeyPath();
}

void FileEncryptionDlgAdapter::PersistEncryptionKeyPath(bool flag)
{
    parent->PersistKeyPath(flag);
}

bool FileEncryptionDlgAdapter::HasKeyFilePassphrase() const
{
    return parent->HasKeyFilePassphrase();
}

void FileEncryptionDlgAdapter::UpdateEncryptionKeyStatus()
{
    //Do nothing
}

FileRequestService &FileEncryptionDlgAdapter::GetFileRequestService()
{
    return parent->GetFileRequestService();
}

void FileEncryptionDlgAdapter::StartLoad(const QString &fileName, const QString &encryptionKeyFile,
                                         std::string &passphrase, EncryptPad::PacketMetadata &metadata, std::string &kf_passphrase)
{
    parent->StartDecryption(fileName, encryptionKeyFile, passphrase, metadata, kf_passphrase);
}

void FileEncryptionDlgAdapter::StartSave(const QString &fileName, std::string &kf_passphrase)
{
    parent->StartEncryption(fileName, kf_passphrase);
}
