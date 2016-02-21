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
#include "packet_composer.h"

class FileDlgAsync : public QObject
{
    Q_OBJECT
private:
    QThread *thread_;
    QString input_file_;
    QString output_file_;
    EncryptPad::PacketMetadata &metadata_;
    EncryptPad::PacketResult result_;
    bool is_encryption_;
    EncryptPad::KeyService *key_service_;
    EncryptPad::KeyService *kf_key_service_;
    std::string passphrase_;
    std::string kf_passphrase_;

public slots:
    // this method is on the background thread
    void DoWork();
signals:
    void UpdateStatus(const QString &text);
    void WorkDone();
public:
    FileDlgAsync(EncryptPad::PacketMetadata &metadata);
    bool get_is_encryption() const
    {
        return is_encryption_;
    }

    const QString &get_input_file() const
    {
        return input_file_;
    }

    const QString &get_output_file() const
    {
        return output_file_;
    }

    EncryptPad::PacketResult get_result() const
    {
        return result_;
    }

    void Set(bool is_encryption, const QString &input_file, const QString &output_file, EncryptPad::KeyService *key_service,
             const std::string &passphrase, EncryptPad::KeyService *kf_key_service, const std::string &kf_passphrase)
    {
        is_encryption_ = is_encryption;
        input_file_ = input_file;
        output_file_ = output_file;
        key_service_ = key_service;
        kf_key_service_ = kf_key_service;
        passphrase_ = passphrase;
        result_ = EncryptPad::PacketResult::None;
        kf_passphrase_ = kf_passphrase;
    }

    void Start();
};

