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
#include "async_load.h"
#include <algorithm>
#include <exception>
#include "plog/Log.h"

AsyncLoad::AsyncLoad(EncryptPadEncryptor::Encryptor &encryptor)
    :thread_(new QThread(this)), metadata_(nullptr), encryptor_(encryptor)
{
    QObject::connect(thread_, SIGNAL(started()), this, SLOT(Load()));
    this->moveToThread(thread_);
}

void AsyncLoad::Load()
{
    try
    {
        load_result_ = encryptor_.Load(file_name_.toUtf8().constData(), file_data_,
                encryption_key_file_.toStdString(), passphrase_.empty() ? nullptr : &passphrase_, metadata_,
                kf_passphrase_.empty() ? nullptr : &kf_passphrase_);

        std::fill(std::begin(passphrase_), std::end(passphrase_), '0');
        std::fill(std::begin(kf_passphrase_), std::end(kf_passphrase_), '0');
        passphrase_.clear();
        kf_passphrase_.clear();

        emit AsyncOperationCompleted();
        thread_->quit();
    }
    catch(const std::exception &ex)
    {
        LOG_ERROR << "Critical error. Exception message: " <<  ex.what();
        throw ex;
    }
}

void AsyncLoad::Set(const QString &file_name, const QString &encryption_key_file, const std::string &passphrase,
        EncryptPad::PacketMetadata &metadata, const std::string &kf_passphrase)
{
    file_name_ = file_name;
    encryption_key_file_ = encryption_key_file;
    load_result_ = EncryptPad::EpadResult::None;
    passphrase_ = passphrase;
    kf_passphrase_ = kf_passphrase;
    metadata_ = &metadata;
}

void AsyncLoad::BeginLoad()
{
    thread_->start();
}
