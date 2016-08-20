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
#include <QtGui>
#include <QtWidgets>
#include "set_password_dialog.h"
#include "common_definitions.h"

SetPassphraseDialog::SetPassphraseDialog(QWidget *parent, const bool enableConfirmation)
    :QDialog(parent, kDefaultWindowFlags)
{
    setWindowModality(Qt::WindowModality::ApplicationModal);
    mConfirmationEnabled = enableConfirmation;
    pwdLabel = new QLabel(tr("&Passphrase:"));
    pwdLineEdit = new QLineEdit;
    pwdLineEdit->setEchoMode(QLineEdit::Password);
    pwdLabel->setBuddy(pwdLineEdit);

    confirmPwdLabel = new QLabel(tr("&Confirm:"));
    confirmPwdLineEdit = new QLineEdit;
    confirmPwdLineEdit->setEchoMode(QLineEdit::Password);
    confirmPwdLabel->setBuddy(confirmPwdLineEdit);
    if(!mConfirmationEnabled)
    {
        HideConfirmation();
    }

    okButton = new QPushButton(tr("&OK"));

    cancelButton = new QPushButton(tr("Cancel"));

    connect(okButton, SIGNAL(clicked()),this, SLOT(okClicked()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(pwdLabel);
    leftLayout->addWidget(confirmPwdLabel);

    QVBoxLayout *middleLayout = new QVBoxLayout;
    middleLayout->addWidget(pwdLineEdit);
    middleLayout->addWidget(confirmPwdLineEdit);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(okButton);
    rightLayout->addWidget(cancelButton);
    rightLayout->addStretch();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(middleLayout);
    mainLayout->addLayout(rightLayout);

    setLayout(mainLayout);

    setWindowTitle(tr("Set Passphrase"));
    setFixedHeight(sizeHint().height());
}

void SetPassphraseDialog::HideConfirmation()
{
    confirmPwdLabel->hide();
    confirmPwdLineEdit->hide();
}

void SetPassphraseDialog::okClicked()
{
    if(mConfirmationEnabled && pwdLineEdit->text() != confirmPwdLineEdit->text())
    {
        QMessageBox::warning(
            this,
            tr("Set Passphrase"),
            tr("The confirmed passphrase does not match"));
        return;
    }
    done(QDialog::Accepted);
}

const QString SetPassphraseDialog::GetValue()
{
    return pwdLineEdit->text();
}
