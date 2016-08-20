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
#include "get_password_or_key_dialog.h"
#include "ui_get_password_or_key_dialog.h"
#include "set_encryption_key.h"
#include "common_definitions.h"

GetPassphraseOrKeyDialog::GetPassphraseOrKeyDialog(QWidget *parent, FileRequestService &fileRequestService_p)
    :
    QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::GetPassphraseOrKeyDialog),
    persistKeyPath(false),
    fileRequestService(fileRequestService_p)
{
    ui->setupUi(this);
}

GetPassphraseOrKeyDialog::~GetPassphraseOrKeyDialog()
{
    delete ui;
}

bool GetPassphraseOrKeyDialog::GetPersistKeyPath() const
{
    return persistKeyPath;
}

bool GetPassphraseOrKeyDialog::IsPassphraseSelected() const
{
    return ui->uiPassphraseRadio->isChecked();
}

QString GetPassphraseOrKeyDialog::GetKeyFilePath() const
{
    return ui->uiKeyFile->text();
}

QString GetPassphraseOrKeyDialog::GetPassphrase() const
{
    return ui->uiPassphrase->text();
}

void GetPassphraseOrKeyDialog::on_actionSelectKeyFile_triggered()
{
    using namespace EncryptPad;
    EncryptionKeySelectionResult selection;
    if(!SetEncryptionKey(this, ui->uiKeyFile->text(), persistKeyPath, fileRequestService, selection))
        return;

    persistKeyPath = selection.persist_key_path;
    ui->uiKeyFile->setText(selection.key_file_path);
}
