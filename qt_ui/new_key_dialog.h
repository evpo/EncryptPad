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
#ifndef NEW_KEY_DIALOG_H
#define NEW_KEY_DIALOG_H

#include <QDialog>
#include "file_request_service.h"

namespace Ui {
class NewKeyDialog;
}

class NewKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewKeyDialog(QWidget *parent, FileRequestService &fileRequestService_p);
    ~NewKeyDialog();

    QString getKey() const;
    bool isKeyInRepository() const;

public slots:
    void browseForKeyPath();
    void accept() override;

private:
    Ui::NewKeyDialog *ui;
    FileRequestService &fileRequestService;

};

#endif // NEW_KEY_DIALOG_H
