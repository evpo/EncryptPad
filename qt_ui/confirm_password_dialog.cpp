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
#include "confirm_password_dialog.h"
#include "ui_confirm_password_dialog.h"
#include <QMessageBox>
#include "common_definitions.h"

ConfirmPasswordDialog::ConfirmPasswordDialog(QWidget *parent) :
    QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::ConfirmPasswordDialog)
{
    ui->setupUi(this);
}

ConfirmPasswordDialog::~ConfirmPasswordDialog()
{
    delete ui;
}

void ConfirmPasswordDialog::invokeOk()
{
    if(ui->uiPassword->text() != ui->uiConfirm->text())
    {
        QMessageBox::warning(
            this,
            "Set Password",
            "The confirmed password does not match");
        return;
    }

    done(QDialog::Accepted);
}

QString ConfirmPasswordDialog::GetPassword() const
{
    return ui->uiPassword->text();
}
