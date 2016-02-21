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
#include <QWidget>
#include <QThread>
#include "encryptor.h"

class PlainTextSwitch : public QObject
{
    Q_OBJECT
signals:
    void UpdateSwitch(bool encrypted);
public:
    PlainTextSwitch(QWidget *parent):QObject(parent){}

    void Update(bool encrypted)
    {
        emit UpdateSwitch(encrypted);
    }
};

class PlainTextFunctor : public EncryptPadEncryptor::EncryptedPlainSwitchFunctor  
{
private:
    PlainTextSwitch &plain_text_switch_;

public:
    PlainTextFunctor(PlainTextSwitch &plain_text_switch):plain_text_switch_(plain_text_switch)
    {}

    virtual void EncryptedPlainSwitchChange(bool encrypted)
    {
        plain_text_switch_.Update(encrypted);
    }
};

