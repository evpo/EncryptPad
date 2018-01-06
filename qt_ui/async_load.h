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
#include <QDebug>
#include <QWidget>
#include <QString>
#include <QThread>
#include "encryptor.h"

typedef unsigned char byte;

class AsyncLoad : public QObject
{
    Q_OBJECT
private:
    QThread *thread_;
    QString encryption_key_file_;
    QString file_name_;
    std::string passphrase_;
    std::string kf_passphrase_;
    EncryptPad::PacketMetadata *metadata_;
    EncryptPadEncryptor::Encryptor &encryptor_;
    Botan::SecureVector<byte> file_data_;
    EncryptPad::EpadResult load_result_;
public slots:
    // this method is on the background thread
    void Load();
signals:
    void UpdateStatus(const QString &text);
    void AsyncOperationCompleted();
public:
    const QString &get_file_name() const
    {
        return file_name_;
    }

    EncryptPad::EpadResult get_load_result() const
    {
        return load_result_;
    }

    const Botan::SecureVector<byte> &get_file_data() const
    {
        return file_data_;
    }

    AsyncLoad(EncryptPadEncryptor::Encryptor &encryptor);

    void Set(const QString &file_name, const QString &encryption_key_file, 
             const std::string &passphrase, EncryptPad::PacketMetadata &metadata,
             const std::string &kf_passphrase);

    void BeginLoad();

    void ClearBuffer()
    {
        file_data_.clear();
    }
};

