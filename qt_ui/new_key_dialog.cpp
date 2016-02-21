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
#include "new_key_dialog.h"
#include "ui_new_key_dialog.h"
#include <QDebug>
#include <QFileDialog>
#include "file_name_helper.h"
#include "common_definitions.h"

NewKeyDialog::NewKeyDialog(QWidget *parent, FileRequestService &fileRequestService_p)
    : QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::NewKeyDialog),
    fileRequestService(fileRequestService_p)
{
    ui->setupUi(this);
}

NewKeyDialog::~NewKeyDialog()
{
    delete ui;
}

QString NewKeyDialog::getKey() const
{
    if(isKeyInRepository())
    {
        return ui->uiRepositoryKeyName->text();
    }
    else
    {
        return ui->uiKeyPath->text();
    }
}

bool NewKeyDialog::isKeyInRepository() const
{
    return ui->uiCreateInRepository->isChecked();
}

void NewKeyDialog::browseForKeyPath()
{
    QString fileName = ui->uiKeyPath->text();

    FileRequestSelection selection = fileRequestService.RequestNewFile(
                this, tr("Save New Key To"),
                fileName,
                GetKeyDialogFilter(),
                nullptr);

    if(selection.cancelled)
        return;

    ui->uiKeyPath->setText(selection.file_name);
}
